#ifndef IFCPARSEHELPER_H
#define IFCPARSEHELPER_H

#include <ifcparse/IfcFile.h>
#include <boost/preprocessor/seq/for_each.hpp>

#include "DataNode.h"

template<typename Schema>
class IfcParseHelper
{
public:

    using IfcProject                          =  typename Schema::IfcProject;
    using IfcObjectDefinition                 =  typename Schema::IfcObjectDefinition;
    using IfcRelContainedInSpatialStructure   =  typename Schema::IfcRelContainedInSpatialStructure;
    using IfcRelAggregates                    =  typename Schema::IfcRelAggregates;
    using IfcRelVoidsElement                  =  typename Schema::IfcRelVoidsElement;
    using IfcBuildingStorey                   =  typename Schema::IfcBuildingStorey;

    using HashRel = std::unordered_map<std::string, std::vector<IfcObjectDefinition*>>;

    void extractRelationship_Contains(IfcParse::IfcFile& ifcFile, HashRel& hashRelContains)
    {
        auto pRelsContains = ifcFile.instances_by_type<IfcRelContainedInSpatialStructure>();
        for (auto pRel: *pRelsContains)
        {
            auto guidRelating = pRel->RelatingStructure()->GlobalId();
            auto pRelatedObjects = pRel->RelatedElements();

            auto& vec = hashRelContains[guidRelating];
            vec.reserve(vec.size() + pRelatedObjects->size());

            for (auto pRelated: *(pRelatedObjects))
                vec.push_back(pRelated) ;
        }
    }

    void extractRelationship_Aggregates(IfcParse::IfcFile& ifcFile, HashRel& hashRelAggregates)
    {
        auto pRelsAggregates = ifcFile.instances_by_type<IfcRelAggregates>();
        for (auto pRel : *pRelsAggregates)
        {
            auto guidRelating = pRel->RelatingObject()->GlobalId();
            auto pRelatedObjects = pRel->RelatedObjects();

            auto& vec = hashRelAggregates[guidRelating];
            vec.reserve(vec.size() + pRelatedObjects->size());

            for (auto pRelated : *(pRelatedObjects))
                vec.push_back(pRelated);
        }
    }

    void extractRelationship_Voids(IfcParse::IfcFile& ifcFile, HashRel& hashRelVoids)
    {
        auto pRelsVoids = ifcFile.instances_by_type<IfcRelVoidsElement>();
        for (auto pRel : *pRelsVoids)
        {
            auto guidRelating = pRel->RelatingBuildingElement()->GlobalId();

            if (auto pRelatedObject = pRel->RelatedOpeningElement())
                hashRelVoids[guidRelating].push_back(pRelatedObject);
        }
    }

    /**
     * @brief createPreviewTree: Extract project structure preview from an ifc file,for example:
     * IfcProject
     *  Building
     *    Storey1
     *      Wall(number of wall objects, object guids)
     *      Door
     *      ...
     *    Storey2
     *      ...
     * @param ifcFile: The input ifc file
     * @return spRootNode: The result preview tree root
     **/

    std::unique_ptr<DataNode::Base> createPreviewTree(IfcParse::IfcFile& ifcFile)
    {
        auto spRootNode = std::make_unique<DataNode::Base>();
        auto pIfcProjects = ifcFile.instances_by_type<IfcProject>();

        HashRel hashRelContains, hashRelAggregates, hashRelVoids;

        extractRelationship_Contains(ifcFile, hashRelContains);
        extractRelationship_Aggregates(ifcFile, hashRelAggregates);
        extractRelationship_Voids(ifcFile, hashRelAggregates);


        //comparation class to compare unique pointer of Storey
        struct UPtrCompare
        {
            bool operator()(const std::unique_ptr<DataNode::Storey>& a,
                            const std::unique_ptr<DataNode::Storey>& b) const
            {
                if (a->m_elevation.has_value() && b->m_elevation.has_value())
                    return a->m_elevation.value() < b->m_elevation.value();
                return a->m_name < b->m_name;
            }
        };

        std::map<DataNode::Base*, std::set<std::unique_ptr<DataNode::Storey>, UPtrCompare> > map_pBuilding_upStoreySet;
        std::map<std::string, DataNode::Storey*> map_guid_pStorey;

        //Build structure tree untill storey:
        //it's a storey -> store it for later use, return
        //it's not a storey -> create child node and add to parent, then process it's related objects recuirsively
        std::function<void(DataNode::Base*, IfcObjectDefinition*)> buildTreeUntillStorey = [&](DataNode::Base* pParentNode, typename Schema::IfcObjectDefinition* pIfcObject) {

            auto type = pIfcObject->declaration().name();
            auto guid = pIfcObject->GlobalId();
            auto name = pIfcObject->Name().value_or("");


            if (pIfcObject->declaration().is(Schema::IfcBuildingStorey::Class()))
            {
                if(auto pIfcStorey = pIfcObject->template as<IfcBuildingStorey>())
                {
                    auto upStorey = make_unique<DataNode::Storey>(guid, name,  pIfcStorey->Elevation());
                    auto pStorey = upStorey.get();

                    map_pBuilding_upStoreySet[pParentNode].insert(std::move(upStorey));
                    map_guid_pStorey[guid] = pStorey;
                }
                return;
            }

            //not a storey
            auto pChildNode = pParentNode->addChild(make_unique<DataNode::IfcObject>(guid, name, type));

            if(hashRelContains.contains(guid))
            {
                for (const auto& pRelatedObject : hashRelContains.at(guid))
                    buildTreeUntillStorey(pChildNode, pRelatedObject);
            }

            if(hashRelAggregates.contains(guid))
            {
                for (const auto& pRelatedObject : hashRelAggregates.at(guid))
                    buildTreeUntillStorey(pChildNode, pRelatedObject);
            }

        };

        for (auto pProject : *pIfcProjects)
            buildTreeUntillStorey(spRootNode.get(), pProject);


        //add the given object's guid to the given storey recursively
        std::function<void(DataNode::Storey*, IfcObjectDefinition*)> addObjectToStorey = [&](DataNode::Storey* pStorey, IfcObjectDefinition* pIfcObject) {

            auto objectGuid = pIfcObject->GlobalId();
            auto type = pIfcObject->declaration().name();
            auto name = pIfcObject->Name().value_or("");

            pStorey->m_objectGuidsNamesByType[type].push_back({objectGuid, name});


            //add related objects to storey recursively
            if(hashRelContains.contains(objectGuid))
                for (const auto& pRelatedObject : hashRelContains.at(objectGuid))
                    addObjectToStorey(pStorey, pRelatedObject);

            if(hashRelAggregates.contains(objectGuid))
                for (const auto& pRelatedObject : hashRelAggregates.at(objectGuid))
                    addObjectToStorey(pStorey, pRelatedObject);

            if(hashRelVoids.contains(objectGuid))
                for (const auto& pRelatedObject : hashRelVoids.at(objectGuid))
                    addObjectToStorey(pStorey, pRelatedObject);
        };

        //for each storey, add its related objects recursively
        for (const auto& pair : map_guid_pStorey)
        {
            const auto& storeyGuid = pair.first;
            auto pStorey = pair.second;

            if(hashRelContains.contains(storeyGuid))
                for (const auto& pRelatedObject : hashRelContains.at(storeyGuid))
                    addObjectToStorey(pStorey, pRelatedObject);

            if(hashRelAggregates.contains(storeyGuid))
                for (const auto& pRelatedObject : hashRelAggregates.at(storeyGuid))
                    addObjectToStorey(pStorey, pRelatedObject);
        }

        //complete structural tree: storey nodes and ifcClass nodes
        for (const auto& pair : map_pBuilding_upStoreySet)
        {
            auto pBuildingNode = pair.first;
            for (const auto& upStorey : pair.second)
            {
                //create storey node and add to building node
                auto pStoreyNode = pBuildingNode->addChild( make_unique<DataNode::IfcObject>(upStorey->m_guid, upStorey->m_name, "IfcBuildingStorey") );

                //create ifcClass nodes and add to storey node
                for (auto pair2 : upStorey->m_objectGuidsNamesByType)
                {
                    auto pClassNode = pStoreyNode->addChild( make_unique<DataNode::IfcClass>(pair2.first, pair2.second.size()) );

                    //create ifcObject nodes of the same ifc class
                    for (const auto& pair_guid_name : pair2.second)
                        pClassNode->addChild( make_unique<DataNode::IfcObject>(pair_guid_name.first, pair_guid_name.second, pair2.first));

                }
            }
        }

        return spRootNode;
    }

};

#endif // IFCPARSEHELPER_H
