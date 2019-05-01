#ifndef INDOORGML_WRITER_H
#define INDOORGML_WRITER_H

#include "IndoorGML_reader.h"
#include "rapidxml/rapidxml_print.hpp"

using namespace IndoorGML;
using namespace rapidxml;

// To store temporary strings until they are written in the xml
std::map<unsigned int, str> tmp_str;
std::map<unsigned int, str> tmp_vec_str;
std::map<unsigned int, xml_node<>*> tmp_xml_node;
unsigned int i_tmp_str = 0, i_tmp_vec_str = 0, i_tmp_xml_node = 0;

void writeCellSpaceMembers(LCC& alcc, xml_document<> &mydoc, xml_node<> *PSF, PrimalSpaceFeatures &aPSF)
{
    for( auto& it_csm : *aPSF.getCellSpaceMember() )
    {
        xml_node<> *csm = mydoc.allocate_node(node_element, "core:cellSpaceMember");
        xml_node<> *cs = mydoc.allocate_node(node_element, it_csm.second.className.c_str());
        cs->append_attribute(mydoc.allocate_attribute("gml:id", it_csm.first.c_str()));

        if ( !(it_csm.second.description == "") )
            cs->append_node( mydoc.allocate_node(node_element, "gml:description", it_csm.second.description.c_str()) );
        if ( !(it_csm.second.name == "") )
            cs->append_node( mydoc.allocate_node(node_element, "gml:name", it_csm.second.name.c_str()) );


        /*<core:cellSpaceGeometry>
              <core:Geometry3D>
                  <gml:Solid gml:id="CG-C1">
                      <gml:exterior>
                          <gml:Shell>
                              <gml:surfaceMember>
                                  <gml:Polygon>
                                      <gml:exterior>
                                          <gml:LinearRing>
                                              <gml:pos srsDimension="3">
                                                  187.69736413705405 98.80529991303698 20.0
                                              </gml:pos>*/

        xml_node<> *gmlshell = mydoc.allocate_node(node_element, "gml:Shell");

        typename LCC::Base::One_dart_per_incident_cell_range<2,3>::iterator
                it = alcc.one_dart_per_incident_cell<2,3>(*it_csm.second.getCellSpaceGeometry()).begin(),
                itend = alcc.one_dart_per_incident_cell<2,3>(*it_csm.second.getCellSpaceGeometry()).end();
        for (; it != itend; it++)
        {
            str posList;
            typename LCC::Base::Dart_of_orbit_range<1>::iterator itr(alcc, it);
            for (; itr.cont(); itr++)
            {
                posList += std::to_string( alcc.point(itr).x() + global_shift_pt.x() );
                posList += " ";
                posList += std::to_string( alcc.point(itr).y() + global_shift_pt.y() );
                posList += " ";
                posList += std::to_string( alcc.point(itr).z() + global_shift_pt.z() );
                posList += " ";
            }
            posList.pop_back(); //Remove the last space
            tmp_vec_str[ i_tmp_vec_str++ ]= posList; // Add the first point to close the loop

            xml_node<> *posL = mydoc.allocate_node(node_element, "gml:posList",  tmp_vec_str[ i_tmp_vec_str-1 ].c_str());
            posL->append_attribute( mydoc.allocate_attribute("srsDimension", "3") );

            xml_node<> *LRing = mydoc.allocate_node(node_element, "gml:LinearRing");
            LRing->append_node( posL );

            xml_node<> *gmlext = mydoc.allocate_node(node_element, "gml:exterior");
            gmlext->append_node( LRing );

            xml_node<> *gmlpoly = mydoc.allocate_node(node_element, "gml:Polygon");
            gmlpoly->append_node( gmlext );

            xml_node<> *gmlsM = mydoc.allocate_node(node_element, "gml:surfaceMember");
            gmlsM->append_node( gmlpoly );

            gmlshell->append_node( gmlsM );
        }

        xml_node<> *gmlext = mydoc.allocate_node(node_element, "gml:exterior");
        gmlext->append_node( gmlshell );

        tmp_str[i_tmp_str++] = "CG-" + it_csm.first;
        xml_node<> *cs_solid = mydoc.allocate_node(node_element, "gml:Solid");
        cs_solid->append_attribute( mydoc.allocate_attribute("gml:id", tmp_str[i_tmp_str-1].c_str()) );
        cs_solid->append_node( gmlext );

        xml_node<> *cs_geom3d = mydoc.allocate_node(node_element, "core:Geometry3D");
        cs_geom3d->append_node( cs_solid );

        xml_node<> *cs_geom = mydoc.allocate_node(node_element, "core:cellSpaceGeometry");
        cs_geom->append_node( cs_geom3d );

        cs->append_node( cs_geom );

        xml_node<> *dual = mydoc.allocate_node(node_element, "core:duality");
        str href = "#" + *it_csm.second.getDuality();
        tmp_str[i_tmp_str++] = (href);
        dual->append_attribute( mydoc.allocate_attribute("xlink:href", tmp_str[i_tmp_str-1].c_str()) );
        cs->append_node( dual );

        for( auto& it_pbb : *it_csm.second.getPartialboundedBy() )
        {
            xml_node<> *partialB = mydoc.allocate_node(node_element, "core:partialboundedBy");
            str href = "#" + it_pbb.first;
            tmp_vec_str[ i_tmp_vec_str++ ] = href;
            partialB->append_attribute( mydoc.allocate_attribute("xlink:href", tmp_vec_str[ i_tmp_vec_str-1 ].c_str()) );
            cs->append_node( partialB );
        }

        if ( !(it_csm.second.naviclass == "") )
            cs->append_node( mydoc.allocate_node(node_element, "navi:class", it_csm.second.naviclass.c_str()) );
        if ( !(it_csm.second.function == "") )
            cs->append_node( mydoc.allocate_node(node_element, "navi:function", it_csm.second.function.c_str()) );
        if ( !(it_csm.second.usage == "") )
            cs->append_node( mydoc.allocate_node(node_element, "navi:usage", it_csm.second.usage.c_str()) );

        csm->append_node( cs );
        PSF->append_node( csm );
    }
}


void writeNodes( xml_document<> &mydoc, xml_node<> * nodes, SpaceLayer &sl )
{
    for( auto& it_sm : *sl.getNodes() )
    {
        xml_node<> * sm = mydoc.allocate_node(node_element, "core:stateMember");
        xml_node<> * st = mydoc.allocate_node(node_element, "core:State");
        st->append_attribute(mydoc.allocate_attribute("gml:id", it_sm.first.c_str()));

        if ( !(it_sm.second.description == "") )
            st->append_node( mydoc.allocate_node(node_element, "gml:description", it_sm.second.description.c_str()) );
        if ( !(it_sm.second.name == "") )
            st->append_node( mydoc.allocate_node(node_element, "gml:name", it_sm.second.name.c_str()) );


        xml_node<> *dual = mydoc.allocate_node(node_element, "core:duality");
        str href = "#" + *it_sm.second.getDuality();
        tmp_str[i_tmp_str++] = (href);
        dual->append_attribute( mydoc.allocate_attribute("xlink:href", tmp_str[i_tmp_str-1].c_str()) );
        st->append_node( dual );

        for( auto& it_cns : *it_sm.second.getConnects() )
        {
            xml_node<> *connects = mydoc.allocate_node(node_element, "core:connects");
            str href = "#" + it_cns.first;
            tmp_vec_str[ i_tmp_vec_str++ ] = href;
            connects->append_attribute( mydoc.allocate_attribute("xlink:href", tmp_vec_str[ i_tmp_vec_str-1 ].c_str()) );
            tmp_xml_node[ i_tmp_xml_node++ ] =  connects;
            st->append_node( tmp_xml_node[ i_tmp_xml_node-1 ] );
        }


        // Get the geometry
        tmp_str[i_tmp_str++] = (str( std::to_string((it_sm.second.getGeometry())->x() + global_shift_pt.x() )
                               + " " + std::to_string((it_sm.second.getGeometry())->y() + global_shift_pt.y() )
                               + " " + std::to_string((it_sm.second.getGeometry())->z() + + global_shift_pt.z()) ) );
        xml_node<> *st_pos = mydoc.allocate_node(node_element, "gml:pos",  tmp_str[i_tmp_str-1].c_str() );
        xml_node<> *st_pt = mydoc.allocate_node(node_element, "gml:Point");
        st_pt->append_node( st_pos );
        xml_node<> *st_geom = mydoc.allocate_node(node_element, "core:geometry");
        st_geom->append_node( st_pt );
        st->append_node( st_geom );

        sm->append_node( st );
        nodes->append_node( sm );
    }
}

void writeEdges( xml_document<> &mydoc, xml_node<> * edges, SpaceLayer &sl )
{
    for( auto& it_edg : *sl.getEdges() )
    {
        xml_node<> * edg = mydoc.allocate_node(node_element, "core:transitionMember");
        xml_node<> * trs = mydoc.allocate_node(node_element, "core:Transition");
        trs->append_attribute(mydoc.allocate_attribute("gml:id", it_edg.first.c_str()));

        if ( !(it_edg.second.description == "") )
            trs->append_node( mydoc.allocate_node(node_element, "gml:description", it_edg.second.description.c_str()) );
        if ( !(it_edg.second.name == "") )
            trs->append_node( mydoc.allocate_node(node_element, "gml:name", it_edg.second.name.c_str()) );

        trs->append_node( mydoc.allocate_node(node_element, "core:weight", it_edg.second.getWeight()->c_str()) );


        for( auto& it_cns : *it_edg.second.getConnects() )
        {
            xml_node<> *connects = mydoc.allocate_node(node_element, "core:connects");
            str href = "#" + it_cns.first;
            tmp_vec_str[ i_tmp_vec_str++ ]= href;
            connects->append_attribute( mydoc.allocate_attribute("xlink:href", tmp_vec_str[ i_tmp_vec_str-1 ].c_str()) );
            tmp_xml_node[ i_tmp_xml_node++ ] = connects;
            trs->append_node( tmp_xml_node[ i_tmp_xml_node-1 ] );
        }

        str posList;
        // Get the geometry
        for( auto& it_pos : *it_edg.second.getGeometry() )
        {
            posList +=  str( std::to_string(it_pos.x())
                             + " " + std::to_string(it_pos.y())
                             + " " + std::to_string(it_pos.z()) );
            posList += " ";
        }
        posList.pop_back(); // to remove the last space

        tmp_str[i_tmp_str++] = (posList);
        xml_node<> *trs_pos = mydoc.allocate_node(node_element, "gml:posList",  tmp_str[i_tmp_str-1].c_str() );
        xml_node<> *trs_lstr = mydoc.allocate_node(node_element, "gml:LineString");
        trs_lstr->append_node( trs_pos );
        xml_node<> *trs_geom = mydoc.allocate_node(node_element, "core:geometry");
        trs_geom->append_node( trs_lstr );
        trs->append_node( trs_geom );

        edg->append_node( trs );
        edges->append_node( edg );
    }
}

void writeSpaceLayers(xml_document<> &mydoc, xml_node<> *MLG, MultiLayeredGraph &aMLG)
{
    for(auto& it_sls : *aMLG.getSpaceLayers())
    {
        xml_node<> * SLs = mydoc.allocate_node(node_element, "core:spaceLayers");
        SLs->append_attribute(mydoc.allocate_attribute("gml:id", it_sls.first.c_str()));

        for( auto& it_slm : *it_sls.second.getSpaceLayer() )
        {
            xml_node<> * SLM = mydoc.allocate_node(node_element, "core:spaceLayerMember");
            xml_node<> * SL = mydoc.allocate_node(node_element, "core:spaceLayer");
            SL->append_attribute(mydoc.allocate_attribute("gml:id", it_slm.first.c_str()));

            xml_node<> * nodes = mydoc.allocate_node(node_element, "core:nodes");
            tmp_str[i_tmp_str++] = (str(it_slm.first + "-nodes"));
            nodes->append_attribute(mydoc.allocate_attribute("gml:id", tmp_str[i_tmp_str-1].c_str()));
            writeNodes(mydoc, nodes, it_slm.second);
            SL->append_node( nodes );

            xml_node<> * edges = mydoc.allocate_node(node_element, "core:edges");
            tmp_str[i_tmp_str++] = (str(it_slm.first + "-edges"));
            edges->append_attribute(mydoc.allocate_attribute("gml:id", tmp_str[i_tmp_str-1].c_str()));
            writeEdges( mydoc, edges, it_slm.second );
            SL->append_node( edges );

            SLM->append_node( SL );
            SLs->append_node( SLM );
        }

        MLG->append_node( SLs );
    }
}


void writeIndoorGML( LCC& alcc, IndoorFeatures& InFt, std::ofstream &outfile )
{
    xml_document<> mydoc;
    xml_node<> *root = mydoc.allocate_node(node_element, "core:IndoorFeatures");
    for(auto& it : *InFt.getheader())
    {
        root->append_attribute(mydoc.allocate_attribute(it.first.c_str(), it.second.c_str()));
    }

    if ( InFt.getprimalSpaceFeatures()->getCellSpaceMember()->size() > 0 )
    {
        xml_node<> *pSF = mydoc.allocate_node(node_element, "core:primalSpaceFeatures");
        xml_node<> *PSF = mydoc.allocate_node(node_element, "core:PrimalSpaceFeatures");
        PrimalSpaceFeatures *aPSF = InFt.getprimalSpaceFeatures();
        str psfID = aPSF->getId();
        PSF->append_attribute(mydoc.allocate_attribute("gml:id", psfID.c_str()));

        writeCellSpaceMembers(alcc, mydoc, PSF, *aPSF);
        // writeCellSpaceBoundaryMembers here
        pSF->append_node( PSF );
        root->append_node( pSF );

        str mlgID;
        if ( InFt.getmultiLayeredGraph()->getSpaceLayers()->size() > 0 )
        {
            xml_node<> *mLG = mydoc.allocate_node(node_element, "core:multiLayeredGraph");
            xml_node<> *MLG = mydoc.allocate_node(node_element, "core:MultiLayeredGraph");
            MultiLayeredGraph *aMLG = InFt.getmultiLayeredGraph();
            mlgID = aMLG->getId();
            MLG->append_attribute(mydoc.allocate_attribute("gml:id", mlgID.c_str()));

            writeSpaceLayers(mydoc, MLG, *aMLG);
            mLG->append_node( MLG );
            root->append_node( mLG );
        }

        mydoc.append_node( root );

        outfile << mydoc;
        std::cout << mydoc;
    }

    // cleaning the containers to ensure export with RapidXML
    i_tmp_str = 0;
    tmp_str.clear();
    i_tmp_vec_str = 0;
    tmp_vec_str.clear();
    i_tmp_xml_node = 0;
    tmp_xml_node.clear();
}

#endif // INDOORGML_WRITER_H
