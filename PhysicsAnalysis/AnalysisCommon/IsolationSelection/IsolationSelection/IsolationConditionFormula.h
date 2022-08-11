// Dear emacs, this is -*- c++ -*-

/*
 Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H

#include <memory>
#include "IsolationSelection/IsolationCondition.h"

// Forward Declaration(s)
class TF1;
class TH3F;

namespace CP {
    class IsolationConditionFormula: public IsolationCondition {
        public:
            IsolationConditionFormula(std::string name, xAOD::Iso::IsolationType isoType, const std::string& cutFunction, bool invertCut = false);
            IsolationConditionFormula(std::string name, std::string isoType, const std::string& cutFunction, bool invertCut = false);
            virtual ~IsolationConditionFormula() {
            }

            IsolationConditionFormula(const IsolationConditionFormula& rhs) = delete;
            IsolationConditionFormula& operator=(const IsolationConditionFormula& rhs) = delete;
       
            virtual bool accept(const xAOD::IParticle& x, std::map<xAOD::Iso::IsolationType, float>* cutValues = 0) override;
            virtual bool accept(const strObj& x, std::map<xAOD::Iso::IsolationType, float>* cutValues = 0) override;
            void getCutValue(const float pt);

        private:
            std::shared_ptr<TF1> m_cutFunction;
            bool m_invertCut;
    };
}
#endif
