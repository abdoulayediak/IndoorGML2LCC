#ifndef INDOORGML2LCC_H
#define INDOORGML2LCC_H

#include "LCC_SpecialOps.h"

std::map<str, Dart_handle> cellspace_dart;

namespace IndoorGML
{

    struct externalReferenceType
    {
        std::string informationSystem,
        name,
        uri;
    };

    class CellSpace;
    class CellSpaceBoundary;

    class State
    {
    public:
        std::string description,
                    name;

        State() : Id("")
        {}
        std::string *getId(){return &Id;}
        std::string *getDuality(){return &Duality;}
        std::map<std::string, bool, cmp_string> *getConnects(){return &Connects;}
        Point_3 *getGeometry(){return &Geometry;}

        void setId(std::string id)
        { Id = id; }
        void setDuality(std::string dlt)
        { Duality = dlt; }
        void setConnects(std::map<std::string, bool, cmp_string> &cns)
        { Connects = cns; }
        void setGeometry(Point_3 pt)
        { Geometry = pt; }

        void addConnects(std::string cn)
        { Connects[cn] = true; }

    private:
        std::string Id;
        std::string Duality;
        std::map<std::string, bool, cmp_string> Connects; // The keys correspond to "Transition" Ids
        Point_3 Geometry;
    };



    class Transition
    {
    public:
        std::string description,
                    name;

        Transition() : Id(""),
                       Weight("1.0")
        {}
        std::string *getId(){return &Id;}
        std::string *getWeight(){return &Weight;}
        std::map<std::string, bool, cmp_string> *getConnects(){return &Connects;}
        std::string *getDuality(){return &Duality;}
        std::vector<Point_3> *getGeometry(){return &Geometry;}

        void setId(std::string id)
        { Id = id; }
        void setWeight( std::string w )
        { Weight = w; }
        void setDuality(std::string dlt)
        { Duality = dlt; }
        void setGeometry(std::vector<Point_3>& geom)
        { Geometry = geom; }

        void addConnects(std::string cn)
        { Connects[cn] = true; }
        void addGeometry( Point_3 pt )
        { Geometry.push_back( pt ); }


    private:
        std::string Id,
                    Weight;
        std::map<std::string, bool, cmp_string> Connects; // Pair of "State" Ids
        std::string Duality;
        std::vector<Point_3> Geometry;
    };



    class CellSpace
    {
    public:

        std::string className, // Tells whether the CellSpace is a General, Transfer, Transition, Connection or AnchorSpace
                    description, // Changes in the 1.0.3 to include information about the storey
                    name, // And name of the storey?
                    naviclass,
                    function,
                    usage;

        CellSpace() : Id(""),
                      isNavigable(true),
                      className("core:CellSpace")
        {}
        std::string *getId(){return &Id;}
        Dart_handle *getCellSpaceGeometry(){return &CellSpaceGeometry;}
        std::string *getDuality(){return &Duality;}
        std::map<std::string, bool, cmp_string> *getPartialboundedBy(){return &PartialboundedBy;}

        void setId(std::string id)
        { Id = id; }
        void setCellSpaceGeometry(Dart_handle d)
        {
            CellSpaceGeometry = d;
            cellspace_dart[ *getId() ] = d;
        }
        void setDuality(std::string dlt)
        { Duality = dlt; }
        void setPartialboundedBy(std::map<std::string, bool, cmp_string> res)
        { PartialboundedBy = res; }

    private:
        std::string Id;
        // Dart of a 3-cell
        Dart_handle CellSpaceGeometry;
        std::string Duality;
        externalReferenceType ExternalReference;
        std::map<std::string, bool, cmp_string> PartialboundedBy; // The keys correspond to CellSpaceBoundary Ids

        // members related to the Navigation module
        bool isNavigable; // Tells if the CellSpace is navigable or not

    };


    class CellSpaceBoundary
    {
    public:
        CellSpaceBoundary() : Id("")
        {}
        std::string getId(){return Id;}
        Dart_handle getCellSpaceGeometry(){return CellSpaceBoundaryGeometry;}
        std::string getDuality(){return Duality;}

        void setId(std::string id)
        { Id = id; }

    private:
        std::string Id;
        // Dart of a 2-cell
        Dart_handle CellSpaceBoundaryGeometry;
        std::string Duality;
        externalReferenceType ExternalReference;
    };


    class SpaceLayer
    {

    public:
        SpaceLayer() : Id("")
        {}
        std::string *getId(){return &Id;}
        std::map<std::string, State, cmp_string> *getNodes(){return &nodes;}
        std::map<std::string, Transition, cmp_string> *getEdges(){return &edges;}

        void setId(std::string id)
        { Id = id; }
        void setEdges( std::map<std::string, Transition, cmp_string>& edg )
        { edges = edg; }

        void addNode( State st )
        { nodes[*(st.getId())] = st; }
        void addTransition ( Transition trs )
        { edges[*(trs.getId())] = trs; }

    private:
        std::string Id,
        Usage,
        TerminationDate,
        Function,
        CreationDate,
        Class;

        std::map<std::string, State, cmp_string> nodes;
        std::map<std::string, Transition, cmp_string> edges;
    };


    class SpaceLayers
    {
    public:
        SpaceLayers() : Id("")
        {}
        std::string *getId(){return &Id;}
        std::map<std::string, SpaceLayer, cmp_string> *getSpaceLayer(){return &spaceLayerMember;}

        void setId(std::string id)
        { Id = id; }
        void setSpaceLayer(std::map<std::string, SpaceLayer, cmp_string>& sl)
        { spaceLayerMember = sl; }

        void addSpaceLayer(SpaceLayer& sl)
        { spaceLayerMember[*(sl.getId())] = sl; }

    private:
        std::string Id;
        std::map<std::string, SpaceLayer, cmp_string> spaceLayerMember;
    };



    class InterLayerConnection
    {

    public:
        InterLayerConnection() : Id("ILC")
        {}
        std::string *getId(){return &Id;}
        std::pair<State*, State*> *getInterConnects(){return &InterConnects;}
        std::pair<SpaceLayer*, SpaceLayer*> *getConnectedLayers(){return &ConnectedLayers;}

        void setId(std::string id)
        { Id = id; }

    private:
        std::string Id,
        typeOfTopoExpression,
        comment;

        std::pair<State*, State*> InterConnects;
        std::pair<SpaceLayer*, SpaceLayer*> ConnectedLayers;
    };


    class MultiLayeredGraph
    {

    public:
        MultiLayeredGraph() : Id("MLG")
        {}
        std::string getId(){return Id;}
        std::map<std::string, SpaceLayers, cmp_string> *getSpaceLayers(){return &spaceLayers;}
        std::map<std::string, InterLayerConnection, cmp_string> *getInterEdges(){return &interEdges;}

        void setId(std::string id)
        { Id = id; }
        void setSpaceLayers(std::map<std::string, SpaceLayers, cmp_string>& sl)
        { spaceLayers = sl; }
        void setInterEdges(std::map<std::string, InterLayerConnection, cmp_string>& ie)
        { interEdges = ie; }


        void addSpaceLayers(SpaceLayers& sl)
        { spaceLayers[*(sl.getId())] = sl; }
        void addInterEdges(InterLayerConnection& ie)
        { interEdges[*(ie.getId())] = ie; }


    private:
        std::string Id;
        std::map<std::string, SpaceLayers, cmp_string> spaceLayers;
        std::map<std::string, InterLayerConnection, cmp_string> interEdges;

    };


    class PrimalSpaceFeatures
    {

    public:
        PrimalSpaceFeatures() : Id("PSF")
        {}
        std::string getId(){return Id;}
        std::map<std::string, CellSpace, cmp_string> *getCellSpaceMember(){return &cellSpaceMember;}
        std::map<std::string, CellSpaceBoundary, cmp_string> *getCellSpaceBoundaryMember(){return &cellSpaceBoundaryMember;}

        void setId(std::string id)
        { Id = id; }
        void setCellSpaceMember(std::map<std::string, CellSpace, cmp_string>& csm)
        { cellSpaceMember = csm; }
        void setCellSpaceMember(std::map<std::string, CellSpaceBoundary, cmp_string>& csbm)
        { cellSpaceBoundaryMember = csbm; }

        void addCellSpaceMember(CellSpace& csm)
        { cellSpaceMember[*(csm.getId())] = csm; }
        void addCellSpaceBoundaryMember(CellSpaceBoundary& csbm)
        { cellSpaceBoundaryMember[csbm.getId()] = csbm; }

    private:
        std::string Id;
        std::map<std::string, CellSpace, cmp_string> cellSpaceMember;
        std::map<std::string, CellSpaceBoundary, cmp_string> cellSpaceBoundaryMember;

    };

    class IndoorFeatures
    {

    public:
        IndoorFeatures() : Id("IFt")
        {
            cellspace_dart.clear();
        }

        std::string getId(){return Id;}
        PrimalSpaceFeatures *getprimalSpaceFeatures(){return &primalSpaceFeatures;}
        MultiLayeredGraph *getmultiLayeredGraph(){return &multiLayeredGraph;}
        std::map<std::string, std::string, cmp_string> *getheader(){return &header;}

        void setId(std::string id)
        { Id = id; }
        void setprimalSpaceFeatures(PrimalSpaceFeatures& psf)
        { primalSpaceFeatures = psf; }
        void setmultiLayeredGraph(MultiLayeredGraph& mlg)
        { multiLayeredGraph = mlg; }

        void addheader(std::pair<std::string, std::string> attrib)
        { header[attrib.first] = attrib.second; }


    private:
        std::string Id;
        PrimalSpaceFeatures primalSpaceFeatures;
        MultiLayeredGraph multiLayeredGraph;

        std::map<std::string, std::string, cmp_string> header;

    };



    /// Navigation classes
//    class NavigableSpace : CellSpace
//    {
//    public:
//        std::string function,
//                    usage;
//    };

//    class GeneralSpace : NavigableSpace
//    {};

//    class TransferSpace : NavigableSpace
//    {};

//    class TransitionSpace : TransferSpace
//    {};

//    class ConnectionSpace : TransferSpace
//    {};

//    class AnchorSpace : TransferSpace
//    {};


//    class NavigableBoundary : CellSpaceBoundary
//    {
//    public:
//        std::string function,
//                    usage;
//    };

//    class TransferBoundary : NavigableSpace
//    {};

//    class ConnectionBoundary : TransferBoundary
//    {};

//    class AnchorBoundary : TransferBoundary
//    {};


}


#endif
