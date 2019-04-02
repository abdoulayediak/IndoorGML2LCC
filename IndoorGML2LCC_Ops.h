#ifndef INDOORGML2LCC_OPS_H
#define INDOORGML2LCC_OPS_H

#include "IndoorGML2LCC.h"
int Layers_num = 0,
    Layer_num = 0;

// Containers to ensure export with RapidXML
std::map<unsigned int, str> tmp_str_ops;
std::map<unsigned int, str_pair > tmp_ordered_pairs;
unsigned int i_tmp_str_ops = 0, i_tmp_ordered_pairs = 0;

str_pair ordered_str( str s1, str s2 )
{
    if ( s1.compare(s2) < 0 )
        return str_pair( s1, s2 );
    else
        return str_pair( s2, s1 );
}

Point getCellCentroid(LCC& alcc, Dart_handle cell)
{
    Bbox_3 bb = LCCtools::Get_Bbox_vol(alcc, cell);
    Point pt( (bb.xmax() + bb.xmin())/2.0,
              (bb.ymax() + bb.ymin())/2.0,
              (bb.zmax() + bb.zmin())/2.0);

    return pt;
}

Point getCellCentroid_face(LCC& alcc, Dart_handle face)
{
    double x = 0.0, y = 0.0, z = 0.0;
    int count = 0;
    typename LCC::Base::Dart_of_orbit_range<1>::iterator itr(alcc, face);
    for (; itr.cont(); itr++)
    {
        x += alcc.point( itr ).x();
        y += alcc.point( itr ).y();
        z += alcc.point( itr ).z();
        count++;
    }

    return Point( x/count, y/count, z/count );
}

IndoorGML::State createState( LCC& alcc, Dart_handle cell, str SL_id, unsigned int i )
{
    IndoorGML::State res;

    if ( cell != LCC::null_handle )
    {
        res.setId( str(SL_id + "_ST" + std::to_string( i )) );
        res.setDuality( alcc.info<3>(cell).id() );

        // Run through all the 2-cells (faces) of cell
        typename LCC::Base::template One_dart_per_incident_cell_range<2,3>::iterator
                it = alcc.one_dart_per_incident_cell<2,3>(cell).begin(),
                itend = alcc.one_dart_per_incident_cell<2,3>(cell).end();
        for (; it != itend; it++)
        {
            // If a 2-cell is 3-sewn to another one, then both cells are linked
            if ( !alcc.is_free(it, 3) )
            {
                tmp_str_ops[ i_tmp_str_ops++ ] = alcc.info<3>(alcc.beta<3>(it)).id();
                tmp_ordered_pairs[ i_tmp_ordered_pairs++ ] = ordered_str( alcc.info<3>(cell).id(), tmp_str_ops[ i_tmp_str_ops-1 ] );
                res.addConnects( str ( "T"
                                       + tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].first + "-"
                                       + tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].second ) );
            }
        }
        res.setGeometry( getCellCentroid(alcc, cell) );
    }

    return res;
}


std::map<str, IndoorGML::Transition, cmp_string> createTransitions( LCC& alcc, str SL_id )
{
    std::map<str, IndoorGML::Transition, cmp_string> res;
    // Run through all the 2-cells (faces) of the LCC
    typename LCC::Base::template One_dart_per_cell_range<2>::iterator it = alcc.one_dart_per_cell<2>().begin(),
                                                             itend = alcc.one_dart_per_cell<2>().end();
    for(; it!=itend; it++)
    {
        IndoorGML::Transition trs;
        if ( !alcc.is_free(it, 3) )
        {
            tmp_str_ops[ i_tmp_str_ops++ ] = alcc.info<3>(alcc.beta<3>(it)).id();
            tmp_ordered_pairs[ i_tmp_ordered_pairs++ ] = ordered_str( alcc.info<3>(it).id(), tmp_str_ops[ i_tmp_str_ops-1 ] );
            str id = str(SL_id + "_T"
                         + tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].first + "-"
                         + tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].second );
            trs.setId( id );

            trs.addConnects( alcc.info<3>(it).id() );
            trs.addConnects( tmp_str_ops[ i_tmp_str_ops-1 ] );

            std::vector<Point_3> geom = { getCellCentroid(alcc, cellspace_dart[ tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].first ] ),
                                          getCellCentroid_face(alcc, it),
                                          getCellCentroid(alcc, cellspace_dart[ tmp_ordered_pairs[ i_tmp_ordered_pairs-1 ].second ] )
                                        };
            trs.setGeometry( geom );
            res[id] = trs;
        }
    }

    return res;
}


IndoorGML::SpaceLayer createSpaceLayer(LCC& alcc, str id)
{
    IndoorGML::SpaceLayer res;
    res.setId( id );

    /*Usage,
    TerminationDate,
    Function,
    CreationDate,
    Class;*/

    unsigned int count = 0;
    typename LCC::Base::One_dart_per_cell_range<3>::iterator it = alcc.one_dart_per_cell<3>().begin(),
                                                             itend = alcc.one_dart_per_cell<3>().end();
    for(; it!=itend; it++)
    {
        res.addNode( createState( alcc, it, id, count++ ) );
    }
    res.setEdges( createTransitions( alcc, id ) );

    return res;
}

void updateCellSpaceDuality( IndoorGML::IndoorFeatures &InFt, IndoorGML::SpaceLayer &SL )
{
    for( auto& it : *SL.getNodes() )
    {
        tmp_str_ops[ i_tmp_str_ops++ ] = *it.second.getDuality();
        // The line below is reeeeally not cool... (clearer alternative commented just below it)
        InFt.getprimalSpaceFeatures()->getCellSpaceMember()->operator[](tmp_str_ops[ i_tmp_str_ops-1 ]).setDuality( *it.second.getId() );

//        std::map<std::string, IndoorGML::CellSpace, cmp_string> *it_csm = InFt.getprimalSpaceFeatures()->getCellSpaceMember();
//        IndoorGML::CellSpace *cs = &((*it_csm)[ tmp_str_ops.back() ]);
//        cs->setDuality( *it.second.getId() );
    }
}

void createNRGLayer( LCC& alcc, IndoorGML::IndoorFeatures &InFt )
{
    // cleaning the containers to ensure export with RapidXML
    tmp_str_ops.clear();
    tmp_ordered_pairs.clear();

    IndoorGML::SpaceLayers SLs;
    SLs.setId( str("NRG" + std::to_string(Layers_num++)) );

    IndoorGML::SpaceLayer SL = createSpaceLayer( alcc, str("SL" + std::to_string(Layer_num++)) );
    updateCellSpaceDuality( InFt, SL );
    SLs.addSpaceLayer( SL );

    InFt.getmultiLayeredGraph()->addSpaceLayers( SLs );
}


#endif
