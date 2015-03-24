# mazerouter
A core program for routing gates/component in ASIC. This program was built based on assumption as follows:
  - all components are connected with 2-points nets connection
  - 2 metal layers available
  
The main program is invoked using this command:
  router.exe <grid-file> <netlist-file>
It reads two file:
  1.  Grid File (.grid)
  
      <Nx>  <Ny>  <bendpenalty> <viapenalty>
      <x0,y0>   <x1,y0>   <x2,y0>   ... <xN-2,y0>   <xN-1,y0>     // Layer-0
      <x0,y1>   <x1,y1>   <x2,y1>   ... <xN-2,y1>   <xN-1,y1>
      ...
      <x0,yN-2> <x1,yN-2> <x2,yN-2> ... <xN-2,yN-2> <xN-1,yN-2>
      <x0,yN-1> <x1,yN-1> <x2,yN-1> ... <xN-2,yN-1> <xN-1,yN-1>
      <x0,y0>   <x1,y0>   <x2,y0>   ... <xN-2,y0>   <xN-1,y0>     // Layer-1
      <x0,y1>   <x1,y1>   <x2,y1>   ... <xN-2,y1>   <xN-1,y1>
      ...
      <x0,yN-2> <x1,yN-2> <x2,yN-2> ... <xN-2,yN-2> <xN-1,yN-2>
      <x0,yN-1> <x1,yN-1> <x2,yN-1> ... <xN-2,yN-1> <xN-1,yN-1>
  
      The first line contains parameter values: grid size, bend penalty and via penalty. The following lines
      represents the cost for each cell in grid.
  
  2.  Netlist File (.nl)
  
      <Nnet> // NetNumber = 3 nets to be routed
      <#1>      <source-layer>  <source-x>  <source-y>  <target-layer>  <target-x>  <target-y>
      <#2>      <source-layer>  <source-x>  <source-y>  <target-layer>  <target-x>  <target-y>
      ...
      <#Nnet>   <source-layer>  <source-x>  <source-y>  <target-layer>  <target-x>  <target-y>
      
     


