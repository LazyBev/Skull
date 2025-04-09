local Lexer = require("lexer")
local Parser = require("parser")
local Codegen = require("codegen")

-- Error handling with location and time
local function formatError(phase, message, file, line, column)
    local time = os.date("%Y-%m-%d %H:%M:%S")
    return string.format("\n[%s] Error during %s\nLocation: %s:%d:%d\nError: %s\n",
        time, phase, file or "unknown", line or 0, column or 0, message)
end

local function logMessage(message)
    print(string.format("[%s] %s", os.date("%H:%M:%S"), message))
end

-- Command line argument handling
local inputFile = nil
local outputFile = nil
local verbose = false
local shouldRun = false
local executableName = nil

-- Process all flags first
local i = 1
while i <= #arg do
    local currentArg = arg[i]
    if currentArg:sub(1, 1) == "-" then
        if currentArg == "-o" then
            if i + 1 <= #arg then
                outputFile = arg[i + 1]
                i = i + 1
            else
                io.stderr:write(formatError("argument parsing", "-o requires an output file name"))
                os.exit(1)
            end
        elseif currentArg == "-v" then
            verbose = true
        elseif currentArg == "-run" then
            shouldRun = true
        else
            io.stderr:write(formatError("argument parsing", "Unknown option: " .. currentArg))
            os.exit(1)
        end
    else
        -- First non-flag argument is the input file
        if not inputFile then
            inputFile = currentArg
        else
            io.stderr:write(formatError("argument parsing", "Multiple input files specified"))
            os.exit(1)
        end
    end
    i = i + 1
end

if not inputFile then
    print("Usage: lua tidal.lua [options] input_file")
    print("Options:")
    print("  -o <file>    Specify output file")
    print("  -v           Verbose output")
    print("  -run         Run the program after compilation")
    print("")
    print("Example usage:")
    print("  lua tidal.lua -v -run test.c")
    print("  lua tidal.lua -o output.asm input.c")
    os.exit(1)
end

-- Create build directory if it doesn't exist
os.execute("mkdir -p build")

-- Set default output names
local baseName = inputFile:match("(.+)%.[^%.]+$") or inputFile
outputFile = outputFile or "build/" .. baseName .. ".asm"
local objectFile = "build/" .. baseName .. ".o"
executableName = "build/" .. baseName

-- Read input file
local file = io.open(inputFile, "r")
if not file then
    io.stderr:write(formatError("file reading", "Could not open input file: " .. inputFile))
    os.exit(1)
end
local source = file:read("*all")
file:close()

if verbose then
    logMessage("Compiling " .. inputFile)
end

local function try(phase, f, ...)
    local status, result = pcall(f, ...)
    if not status then
        io.stderr:write(formatError(phase, result))
        os.exit(1)
    end
    return result
end

-- Create lexer and tokenize
local lexer = Lexer.new(source, inputFile)
local tokens = try("lexical analysis", lexer.tokenize, lexer)

if verbose then
    logMessage("Tokenization complete")
end

-- Parse tokens into AST
local parser = Parser.new(lexer)
local ast = try("parsing", parser.parse, parser)

if verbose then
    logMessage("Parsing complete")
end

-- Generate assembly code
local codegen = Codegen.new()
codegen:loadSource(inputFile, source)
local assembly = try("code generation", function()
    codegen:generate(ast)
    return codegen:getOutput()
end)

if verbose then
    logMessage("Code generation complete")
end

-- Write assembly output file
local out = io.open(outputFile, "w")
if not out then
    io.stderr:write(formatError("file writing", "Could not open output file: " .. outputFile))
    os.exit(1)
end
out:write(assembly)
out:close()

if verbose then
    logMessage("Assembly written to " .. outputFile)
end

-- Assemble and link
if verbose then
    logMessage("Assembling with NASM...")
end

local nasmResult = os.execute("nasm -f elf64 -o " .. objectFile .. " " .. outputFile)
if not nasmResult then
    io.stderr:write(formatError("assembly", "NASM assembly failed"))
    os.exit(1)
end

if verbose then
    logMessage("Linking...")
end

local ldResult = os.execute("ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -o " .. executableName .. " " .. objectFile)
if not ldResult then
    io.stderr:write(formatError("linking", "Linking failed"))
    os.exit(1)
end

if verbose then
    logMessage("Executable created: " .. executableName)
end

-- Run the program if requested
if shouldRun then
    if verbose then
        logMessage("Running the program...")
        print("------- Program Output -------")
    end
    local runResult = os.execute("./" .. executableName)
    if not runResult then
        io.stderr:write(formatError("execution", "Program execution failed"))
        os.exit(1)
    end
    if verbose then
        print("------- Program End -------")
    end
end 