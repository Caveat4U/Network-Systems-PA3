Worked in conjunction with Chris Fichmann at the very tail end of this project.

When starting a router, simply pass in its name, log filename and initialization filename in the command
line, e.g. `./routed_LS <RouterID> <LogFileName> <Initialization file>` to start the router.

I'm using sleep() in order to allow all the routers to come online and prevent unnecessary hanging.

For the graceful exit, on any router once it's started sending/receiving packets (after it's connected), you can press CTRL+C. It will send an exit notification to all other routers, and everything should shut down. Give it at least 10 seconds. It sometimes takes a while for everything to propagate through.
