
librapl
=======

Librapl is a library that simplifies access to the RAPL values in
MSR registers of modern Intel CPUs, e.g., SandyBridge or IvyBridge
processors. It also contains a sample application that can either
print the current energy consumption on the console or write the
values with a Gnuplot-friendly format in a file. Currently it 
provides the consumption of the package, the CPU and GPU as well
as peripheral components (uncore).

See [kicherer.org](https://kicherer.org/joomla/index.php/de/blog/liste/31-measuring-energy-consumption-with-librapl-on-sandy-ivybridge-processors)
for an example created with Gnuplot.

Status
------

Currently, the library only provides an interface for reading the energy
consumption and it is only tested with an IvyBridge CPU. Please send me an
email or open an issue if you got results with SandyBridge or newer CPUs.
