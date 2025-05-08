# 2025-05-07T16:36:51.868840300
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze")
status = platform.build()

status = platform.build()

comp = client.get_component(name="main")
comp.build()

