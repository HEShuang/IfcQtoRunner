#ifndef IFCSCHEMA_STRATEGY_IMPL_H
#define IFCSCHEMA_STRATEGY_IMPL_H

#include "IfcSchemaStrategyBase.h"
#include <ifcparse/IfcFile.h>
#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x1.h>
#include <ifcparse/Ifc4x2.h>
#include <ifcparse/Ifc4x3.h>
#include <ifcparse/Ifc4x3_add2.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3.h>

template<typename Schema>
class IfcSchemaStrategyImpl : public IfcSchemaStrategyBase {
private:
    IfcParse::IfcFile& m_ifcFile;

public:
    IfcSchemaStrategyImpl(IfcParse::IfcFile& file) : m_ifcFile(file) {}

    void extractRelationship_Contains(IfcParse::IfcFile& ifcFile, HashRel& hashRelContains) const override {
        auto pRelsContains = ifcFile.instances_by_type<typename Schema::IfcRelContainedInSpatialStructure>();
        for (auto pRel: *pRelsContains) {
            auto guidRelating = pRel->RelatingStructure()->GlobalId();
            auto pRelatedObjects = pRel->RelatedElements();

            auto& vec = hashRelContains[guidRelating];
            vec.reserve(vec.size() + pRelatedObjects->size());

            for (auto pRelated: *(pRelatedObjects))
                vec.push_back(pRelated) ;
        }
    }

    void extractRelationship_Aggregates(IfcParse::IfcFile& ifcFile, HashRel& hashRelAggregates) const override {
        auto pRelsAggregates = ifcFile.instances_by_type<typename Schema::IfcRelAggregates>();
        for (auto pRel : *pRelsAggregates) {
            auto guidRelating = pRel->RelatingObject()->GlobalId();
            auto pRelatedObjects = pRel->RelatedObjects();

            auto& vec = hashRelAggregates[guidRelating];
            vec.reserve(vec.size() + pRelatedObjects->size());

            for (auto pRelated : *(pRelatedObjects))
                vec.push_back(pRelated);
        }
    }

    void extractRelationship_Voids(IfcParse::IfcFile& ifcFile, HashRel& hashRelVoids) const override {
        auto pRelsVoids = ifcFile.instances_by_type<typename Schema::IfcRelVoidsElement>();
        for (auto pRel : *pRelsVoids) {
            auto guidRelating = pRel->RelatingBuildingElement()->GlobalId();

            if (auto pRelatedObject = pRel->RelatedOpeningElement())
                hashRelVoids[guidRelating].push_back(pRelatedObject);
        }
    }

    void getProjects(IfcParse::IfcFile& file, std::vector<IfcUtil::IfcBaseClass*>& ifcProjects) const override {

        if (auto pContainer = file.instances_by_type<typename Schema::IfcProject>()) {
            for (auto p : *pContainer) {
                ifcProjects.push_back(p);
            }
        }
    }

    bool isStorey(IfcUtil::IfcBaseClass* obj) const override {
        return obj->declaration().is(Schema::IfcBuildingStorey::Class());
    }

    // --- Property Accessors ---

    std::string getGlobalId(IfcUtil::IfcBaseClass* obj) const override {
        if(auto pIfcRoot = obj->as<typename Schema::IfcRoot>())
            return pIfcRoot->GlobalId();
        return "";
    }

    std::string getName(IfcUtil::IfcBaseClass* obj) const override {
        if(auto pIfcRoot = obj->as<typename Schema::IfcRoot>())
            return pIfcRoot->Name().value_or("");
        return "";
    }

    std::string getTypeName(IfcUtil::IfcBaseClass* obj) const override {
        return obj->declaration().name();
    }

    std::optional<double> getStoreyElevation(IfcUtil::IfcBaseClass* obj) const override {

        if (auto pStorey = obj->as<typename Schema::IfcBuildingStorey>()) {
            auto elevation = pStorey->Elevation();
            return elevation ? std::make_optional(*elevation) : std::nullopt;
        }
        return std::nullopt;
    }

};

#endif // IFCSCHEMA_STRATEGY_IMPL_H
