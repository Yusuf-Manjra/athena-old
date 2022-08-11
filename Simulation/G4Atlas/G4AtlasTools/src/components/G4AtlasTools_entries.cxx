#include "G4AtlasTools/DetectorGeometryBase.h"
#include "G4AtlasTools/CylindricalEnvelope.h"
#include "G4AtlasTools/BoxEnvelope.h"
#include "G4AtlasTools/PolyconicalEnvelope.h"
#include "G4AtlasTools/G4AtlasDetectorConstructionTool.h"
#include "G4AtlasTools/RegionCreator.h"
#include "G4AtlasTools/AddPhysicsDecayTool.h"
#include "../SensitiveDetectorMasterTool.h"
#include "../FastSimulationMasterTool.h"
#include "G4AtlasTools/GlobalFieldManagerTool.h"
#include "G4AtlasTools/DetectorFieldManagerTool.h"
#include "../G4ThreadInitTool.h"
#include "../MaterialDescriptionTool.h"
#include "../VoxelDensityTool.h"

DECLARE_COMPONENT( DetectorGeometryBase )
DECLARE_COMPONENT( CylindricalEnvelope )
DECLARE_COMPONENT( BoxEnvelope )
DECLARE_COMPONENT( PolyconicalEnvelope )
DECLARE_COMPONENT( G4AtlasDetectorConstructionTool )
DECLARE_COMPONENT( RegionCreator )
DECLARE_COMPONENT( SensitiveDetectorMasterTool )
DECLARE_COMPONENT( FastSimulationMasterTool )
DECLARE_COMPONENT( AddPhysicsDecayTool )
DECLARE_COMPONENT( GlobalFieldManagerTool )
DECLARE_COMPONENT( DetectorFieldManagerTool )
DECLARE_COMPONENT( G4ThreadInitTool )
DECLARE_COMPONENT( MaterialDescriptionTool )
DECLARE_COMPONENT( VoxelDensityTool )
