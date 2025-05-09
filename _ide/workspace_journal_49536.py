# 2025-05-08T00:34:10.332666
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.create_platform_component(name = "microblaze3",hw_design = "$COMPONENT_LOCATION/../../../Downloads/microblazeV3.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0")

comp = client.create_app_component(name="Project2",platform = "$COMPONENT_LOCATION/../microblaze3/export/microblaze3/microblaze3.xpfm",domain = "standalone_microblaze_0")

