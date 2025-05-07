# 2025-05-06T18:33:55.490386600
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

comp = client.get_component(name="Project2")
comp.build()

comp.build()

comp.build()

comp.build()

platform = client.create_platform_component(name = "microblaze3",hw_design = "$COMPONENT_LOCATION/../../../Downloads/microblazeV3.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0")

platform = client.get_component(name="microblaze3")
status = platform.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

client.delete_component(name="Project2")

comp = client.create_app_component(name="main",platform = "$COMPONENT_LOCATION/../microblaze3/export/microblaze3/microblaze3.xpfm",domain = "standalone_microblaze_0")

status = platform.build()

comp = client.get_component(name="main")
comp.build()

status = platform.build()

comp.build()

client.delete_component(name="main")

platform = client.create_platform_component(name = "microblaze",hw_design = "$COMPONENT_LOCATION/../../../Downloads/microblazeV3.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0")

comp = client.create_app_component(name="main",platform = "$COMPONENT_LOCATION/../microblaze/export/microblaze/microblaze.xpfm",domain = "standalone_microblaze_0")

platform = client.get_component(name="microblaze")
status = platform.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

vitis.dispose()

platform = client.get_component(name="microblaze")
status = platform.build()

comp = client.get_component(name="main")
comp.build()

status = platform.build()

comp.build()

vitis.dispose()

vitis.dispose()

