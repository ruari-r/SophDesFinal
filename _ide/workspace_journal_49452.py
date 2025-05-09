# 2025-05-09T10:36:05.298671700
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze3")
status = platform.build()

comp = client.get_component(name="Project2")
comp.build()

vitis.dispose()

