# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addService

addService("LArG4Barrel.LArG4BarrelConfigLegacy.getBarrelCryostatCalibrationCalculator","BarrelCryostatCalibrationCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getBarrelCryostatCalibrationLArCalculator","BarrelCryostatCalibrationLArCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getBarrelCryostatCalibrationMixedCalculator","BarrelCryostatCalibrationMixedCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getDMCalibrationCalculator","DMCalibrationCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getBarrelCalibrationCalculator","BarrelCalibrationCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getBarrelPresamplerCalibrationCalculator","BarrelPresamplerCalibrationCalculator")

addService("LArG4Barrel.LArG4BarrelConfigLegacy.getEMBCalculator","EMBCalculator")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getEMBPresamplerCalculator","EMBPresamplerCalculator")

addService("LArG4Barrel.LArG4BarrelConfigLegacy.getLArBarrelGeometry","LArBarrelGeometry")
addService("LArG4Barrel.LArG4BarrelConfigLegacy.getLArBarrelPresamplerGeometry","LArBarrelPresamplerGeometry")
