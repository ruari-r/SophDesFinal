# 2025-05-07T16:26:22.685929900
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze")
status = platform.build()

comp = client.get_component(name="main")
comp.build()

vitis.dispose()

