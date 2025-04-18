# Skull
Skull is a language that offers two things:  
**Graveyard**: *LSC Manager*  
**LSC**: *Skull Lang Compiler*  

## LSC
**LSC** stands for *Lazy's Skull Compiler*, it compiles skull files into x86_64 assembly.

## Graveyard
**Graveyard** is a **LSC** compiler, but you can also with **Graveyard** install **LSC** to /usr/bin for systemwide use

# Usage

## Gravyard usage
```bash
                        Graveyard - Usage Information                        
-----------------------------------------------------------------------------

Usage: graveyard <Target>

Targets:
        diff          : Checks for any diffs in bytes between the Installed Graveyard and the Graveyard in the Skull dir
        resurrect     : Updates Graveyard or if you dont have it installed it Installs Graveyard to /usr/bin
        lsc-compile   : Compiles LSC to a binary
        lsc-remove    : Deletes the compiled LSC binary and its build artifacts
        lsc-recompile : Recompiles LSC (Alternative: Graveyard lsc-remove && Graveyard lsc-compile)
        lsc-install   : Compiles LSC and installs to /usr/bin
        lsc-uninstall : Uninstalls LSC from /usr/bin
        lsc-reinstall : Reinstalls LSC (Alternative: Graveyard lsc-uninstall && Graveyard lsc-install)
        usage         : Display this help message
```

## How to compile LSC with Graveyard

This line *Installs/Updates* **Graveyard**
```bash
graveyard resurrect
```

This line *compiles* **LSC**  
```bash
graveyard lsc-compile
```

If you want to *install* **LSC** systemwide
```bash
graveyard lsc-install
```

## LSC Usage
```bash
Usage: lsc [options] input_file.k

Options:
        -o, --output FILE    Specify output executable name
        -k, --keep-files     Keep intermediate .asm and .o files
        -h, --help           Show this help message
```

## How to compile Skull with LSC

Outputs to "main" executable
```bash
lsc <filename.k>
```

Explicitly name the output
```bash
lsc <filename.k> -o <output>
```

Keeps .o and .asm file from being deleted, this allows me to debug and you to see the inner workings :D
```bash
lsc <filename.k> -k
```

Explicitly name the output AND keep files
```bash
lsc <filename.k> -o <output> -k
```

Or you can do it this way
```bash
lsc <filename.k> -k -o <output>
```
