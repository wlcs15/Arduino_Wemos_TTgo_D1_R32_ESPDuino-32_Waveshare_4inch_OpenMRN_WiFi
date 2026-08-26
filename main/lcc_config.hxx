#ifndef D1R32_LCC_CONFIG_HXX_
#define D1R32_LCC_CONFIG_HXX_

#include "openlcb/ConfigRepresentation.hxx"
#include "openlcb/MemoryConfig.hxx"

namespace openlcb
{

static constexpr uint16_t CANONICAL_VERSION = 0x2001;

CDI_GROUP(NodeSegment, Segment(MemoryConfigDefs::SPACE_CONFIG), Offset(128));
CDI_GROUP_ENTRY(internal_config, InternalConfigData);
CDI_GROUP_END();

CDI_GROUP(ConfigDef, MainCdi());
CDI_GROUP_ENTRY(ident, Identification);
CDI_GROUP_ENTRY(acdi, Acdi);
CDI_GROUP_ENTRY(userinfo, UserInfoSegment, Name("User Info"));
CDI_GROUP_ENTRY(seg, NodeSegment, Name("Settings"));
CDI_GROUP_END();

} // namespace openlcb

#endif
