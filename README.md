# Skull
A programming language that uses LSC

## LSC
LSC stands for Lazy's Skull Compiler, it compiles skull into x86_64 assembly.

### How to Use LSC
Use Graveyard to compile LSC

## Graveyard
Graveyard is a LSC compiler, but you can also with Graveyard install LSC to /usr/bin for systemwide use

### Gravyard usage
```bash
                        Graveyard - Usage Information                        
-----------------------------------------------------------------------------

Usage: Graveyard <Target>
                  diff          : Checks for a diff in bytes between Installed Graveyard and Graveyard in Skull
                  resurrect     : Installs Graveyard to /usr/bin and Updates
                  lsc-compile   : Compiles LSC to a binary
                  lsc-remove    : Deletes the compiled LSC binary and its' build artifacts
                  lsc-recompile : Recompiles LSC (Alternative: Graveyard lsc-remove && Graveyard lsc-compile)
                  lsc-install   : Compiles LSC and installs to /usr/bin
                  lsc-uninstall : Uninstalls LSC from /usr/bin
                  lsc-reinstall : Reinstalls LSC (Alternative: Graveyard lsc-uninstall && Graveyard lsc-install)
                  usage         : Display this help message
```

# How to compile Skull with LSC
```bash
lsc <filename.k> -o <output> // Explicitly name the output
lsc <filename.k> // Implicitly becomes main
lsc <filename.k> -k // Keeps .o and .asm file from being deleted, this allows me to debug and you to see the inner workings :D
lsc <filename.k> -o <output> -k // Combine them
lsc <filename.k> -k -o <output> // Even like this
```
