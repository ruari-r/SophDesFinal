# 2025-05-05T13:54:20.949731900
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze3")
status = platform.build()

comp = client.get_component(name="Project2")
comp.build()

