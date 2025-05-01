# 2025-04-30T14:28:46.129733700
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.create_platform_component(name = "microblaze3",hw_design = "$COMPONENT_LOCATION/../../../Downloads/microblazeV3.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0")

platform = client.get_component(name="microblaze3")
status = platform.build()

comp = client.create_app_component(name="Project2",platform = "$COMPONENT_LOCATION/../microblaze3/export/microblaze3/microblaze3.xpfm",domain = "standalone_microblaze_0")

vitis.dispose()

vitis.dispose()

