/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file PixelDigitization/PixelHistoConverter.h
 * @author Tomas Dado <tomas.dado@cern.ch>
 * @date February, 2021
 * @brief A helper class to conver ROOT TH* objects into simple vectors for speed up
 */

#ifndef PIXELCONDITIONSDATA_PIXELHISTOCONVRTER_H
#define PIXELCONDITIONSDATA_PIXELHISTOCONVRTER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/StatusCode.h"

#include <vector>

/// forward class declaration
class TAxis;
class TH1;
class TH2;
class TH3;

class PixelHistoConverter{
public:
  PixelHistoConverter();
  virtual ~PixelHistoConverter() = default;

  StatusCode setHisto1D(const TH1* histo);
  StatusCode setHisto2D(const TH2* histo);
  StatusCode setHisto3D(const TH3* histo);

  inline float getContent(std::size_t x) const {
    return m_content[x];
  }
  inline float getContent(std::size_t x, std::size_t y) const {
    return m_content[x + y*(m_xAxis.nBins)];
  }
  inline float getContent(std::size_t x, std::size_t y, std::size_t z) const {
    return m_content[x + m_xAxis.nBins*(y + (m_yAxis.nBins * z))];
  }

  inline bool isOverflowZ(const float value) const {
    return (value >= m_zAxis.max);
  }
  bool isFirstZ(const float value) const;

private:
  struct Axis {
    std::size_t nBins;
    float min;
    float max;
    float width;
  };

  Axis m_xAxis{};
  Axis m_yAxis{};
  Axis m_zAxis{};

  std::vector<float> m_content;

  static bool setAxis(Axis& axis, const TAxis* rootAxis);

  inline std::size_t findBin(const Axis& axis, const float value) const {
    if (value <= axis.min) return 0;
    if (value >= axis.max) return (axis.nBins - 1);
    return ((value - axis.min) * axis.width);
  }

public:

  template<typename... Args>
  std::size_t getBinX(Args&& ...args) const {
    return findBin(m_xAxis, std::forward<Args>(args)...);
  }

  template<typename... Args>
  std::size_t getBinY(Args&& ...args) const {
    return findBin(m_yAxis, std::forward<Args>(args)...);
  }

  template<typename... Args>
  std::size_t getBinZ(Args&& ...args) const {
    return findBin(m_zAxis, std::forward<Args>(args)...);
  }
};

#endif
