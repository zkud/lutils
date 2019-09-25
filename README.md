# lutils
Linux utils I created studying interfaces and peripherals.

## lspci.py :fire:
Prints the all pci devices
with <code>python lspci.py</code>

## setttl.py :fire:
Sets / Prints ttl [wiki](https://en.wikipedia.org/wiki/Time_to_live) <br>
Usage: <br>
  <code>python setttl.py</code> or <code>python setttl.py help</code> print help <br>
  <code>python setttl.py old</code> print ttl <br>
  <code>python setttl.py restore</code> set old ttl <br>
  <code>python setttl.py value:int</code> set ttl as value
  
## lshdd :fire:
Prints the all hdd devices <br>
First compile with <code>gcc -o lshdd lshdd.c </code>
Second run with <code>./lshdd</code>
