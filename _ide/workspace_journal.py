# 2025-05-05T02:21:49.806982200
import vitis

client = vitis.create_client()
client.set_workspace(path="Project2")

platform = client.get_component(name="microblaze3")
status = platform.build()

comp = client.get_component(name="Project2")
comp.build()

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

status = platform.build()

comp.build()

status = platform.build()

comp.build()

