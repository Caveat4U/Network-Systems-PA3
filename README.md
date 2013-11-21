When starting a single router, simply pass in its name, log filename 
and initialization filename in the command line, e.g. 

"./routed_LS <RouterID> <LogFileName> <Initialization file>" 
to start the router.

To run all of the routers, the inputting "./run_time" at the command
line will automatically open terminals for each router.

We used sleep() in order to allow all the routers to come online and
prevent unnecessary hanging.
