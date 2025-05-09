# 2025-05-08T14:29:42.330903500
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze3")
status = platform.build()

status = platform.build()

status = platform.build()

status = platform.build()

comp = client.get_component(name="Project2")
comp.build()

status = platform.build()

client.delete_component(name="microblaze3")

platform = client.create_platform_component(name = "microblaze3",hw_design = "$COMPONENT_LOCATION/../../../Downloads/microblazeV3.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0")

status = platform.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

vitis.dispose()

