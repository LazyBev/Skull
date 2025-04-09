local AST = require("ast")

local Codegen = {}
Codegen.__index = Codegen

function Codegen.new()
    return setmetatable({
        output = {},
        currentFunctionName = nil,
        localVars = {},
        localVarCount = 0,
        stringLiterals = {},
        stringLiteralCount = 0,
        source = nil,
        sourceLines = nil,
        sourcePath = nil,
        tempRegisters = {
            "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", "r12", "r13", "r14"
        },
        currentRegisterIndex = 1
    }, Codegen)
end

function Codegen:loadSource(path, source)
    self.sourcePath = path
    self.source = source
    self.sourceLines = {}
    for line in source:gmatch("[^\r\n]+") do
        table.insert(self.sourceLines, line)
    end
end

function Codegen:getRegister()
    local reg = self.tempRegisters[self.currentRegisterIndex]
    self.currentRegisterIndex = (self.currentRegisterIndex % #self.tempRegisters) + 1
    return reg
end

function Codegen:emit(line)
    table.insert(self.output, line)
end

function Codegen:emitComment(comment)
    table.insert(self.output, "    ; " .. comment)
end

function Codegen:emitSourceComment(node)
    if node and node.line and self.sourceLines and self.sourceLines[node.line] then
        local originalLine = self.sourceLines[node.line]:gsub("%s+", " "):sub(1, 40)
        self:emitComment("Line " .. node.line .. ": " .. originalLine)
    end
end

function Codegen:getOutput()
    return table.concat(self.output, "\n")
end

function Codegen:generate(node)
    self:emit("section .data")
    self:emit("")
    self:emit("section .bss")
    self:emit("")
    self:emit("section .text")
    self:emit("global _start")
    self:emit("extern malloc")
    self:emit("extern free")
    self:emit("")

    -- Helper functions
    self:generateHelperFunctions()

    -- Generate the code for the program
    self:generateNode(node)

    -- Add program entry point
    self:emit("_start:")
    self:emit("    push rbp")
    self:emit("    mov rbp, rsp")
    self:emit("    sub rsp, 128")
    
    -- Find main function and generate call to it
    local mainFunctionNode = self:findMainFunction(node)
    if mainFunctionNode then
        self:generateFunctionCall({
            name = "main",
            args = {}
        })
        self:emit("    mov rdi, rax")
        self:emit("    mov rsp, rbp")
        self:emit("    pop rbp")
        self:emit("    mov rax, 60")
        self:emit("    syscall")
    else
        self:emit("    mov rax, 60")
        self:emit("    mov rdi, 0")
        self:emit("    syscall")
    end
end

function Codegen:generateHelperFunctions()
    -- strlen function
    self:emit("strlen:")
    self:emit("    push rbp")
    self:emit("    mov rbp, rsp")
    self:emit("    xor rax, rax")
    self:emit(".strlen_loop:")
    self:emit("    cmp byte [rdi + rax], 0")
    self:emit("    je .strlen_done")
    self:emit("    inc rax")
    self:emit("    jmp .strlen_loop")
    self:emit(".strlen_done:")
    self:emit("    pop rbp")
    self:emit("    ret")
    self:emit("")

    -- int_to_string function for converting integers to strings
    self:emit("int_to_string:")
    self:emit("    push rbp")
    self:emit("    mov rbp, rsp")
    self:emit("    mov rax, rsi")
    self:emit("    mov rsi, rdi")
    self:emit("    mov r8, 0")
    self:emit("    mov r9, 10")
    self:emit("    test rax, rax")
    self:emit("    jns .positive")
    self:emit("    neg rax")
    self:emit("    mov byte [rsi], '-'")
    self:emit("    inc rsi")
    self:emit("    inc r8")
    self:emit(".positive:")
    self:emit("    mov rdi, rsi")
    self:emit(".convert_loop:")
    self:emit("    xor rdx, rdx")
    self:emit("    div r9")
    self:emit("    add dl, '0'")
    self:emit("    mov [rsi], dl")
    self:emit("    inc rsi")
    self:emit("    inc r8")
    self:emit("    test rax, rax")
    self:emit("    jnz .convert_loop")
    self:emit("    mov byte [rsi], 10")
    self:emit("    inc r8")
    self:emit("    mov rsi, rdi")
    self:emit("    dec rsi")
    self:emit(".reverse_loop:")
    self:emit("    cmp rsi, rdi")
    self:emit("    jge .done")
    self:emit("    mov al, [rsi+1]")
    self:emit("    mov bl, [rdi]")
    self:emit("    mov [rsi+1], bl")
    self:emit("    mov [rdi], al")
    self:emit("    inc rsi")
    self:emit("    dec rdi")
    self:emit("    jmp .reverse_loop")
    self:emit(".done:")
    self:emit("    mov rax, r8")
    self:emit("    mov rsp, rbp")
    self:emit("    pop rbp")
    self:emit("    ret")
    self:emit("")
end

function Codegen:findMainFunction(node)
    if node.type == AST.NODE_TYPES.PROGRAM then
        for _, statement in ipairs(node.statements) do
            if statement.type == AST.NODE_TYPES.FUNCTION_DECL and statement.name == "main" then
                return statement
            end
        end
    end
    return nil
end

function Codegen:generateNode(node)
    if not node then return end

    if node.type == AST.NODE_TYPES.PROGRAM then
        for _, statement in ipairs(node.statements) do
            self:generateNode(statement)
        end
    elseif node.type == AST.NODE_TYPES.FUNCTION_DECL then
        self:generateFunctionDecl(node)
    elseif node.type == AST.NODE_TYPES.BLOCK then
        self:generateBlock(node)
    elseif node.type == AST.NODE_TYPES.DECLARATION then
        self:generateDeclaration(node)
    elseif node.type == AST.NODE_TYPES.ASSIGNMENT then
        self:generateAssignment(node)
    elseif node.type == AST.NODE_TYPES.RETURN_STMT then
        self:generateReturn(node)
    elseif node.type == AST.NODE_TYPES.IF_STMT then
        self:generateIf(node)
    elseif node.type == AST.NODE_TYPES.WHILE_STMT then
        self:generateWhile(node)
    elseif node.type == AST.NODE_TYPES.FOR_STMT then
        self:generateFor(node)
    elseif node.type == AST.NODE_TYPES.BINARY_EXPR then
        self:generateBinaryExpr(node)
    elseif node.type == AST.NODE_TYPES.UNARY_EXPR then
        self:generateUnaryExpr(node)
    elseif node.type == AST.NODE_TYPES.LITERAL then
        self:generateLiteral(node)
    elseif node.type == AST.NODE_TYPES.STRING_LITERAL then
        self:generateStringLiteral(node)
    elseif node.type == AST.NODE_TYPES.IDENTIFIER then
        self:generateIdentifier(node)
    elseif node.type == AST.NODE_TYPES.FUNCTION_CALL then
        self:generateFunctionCall(node)
    end
end

function Codegen:generateFunctionDecl(node)
    self.currentFunctionName = node.name
    self.localVars = {}
    self.localVarCount = 0
    
    self:emit(node.name .. ":")
    self:emit("    push rbp")
    self:emit("    mov rbp, rsp")
    
    -- Generate the function body
    self:generateNode(node.body)
    
    -- If it's not the main function, add a default return
    if node.name ~= "main" then
        self:emit("    mov rax, 0")
    end
    
    self:emit("    mov rsp, rbp")
    self:emit("    pop rbp")
    self:emit("    ret")
    self:emit("")
end

function Codegen:generateBlock(node)
    for _, statement in ipairs(node.statements) do
        self:emitSourceComment(statement)
        self:generateNode(statement)
    end
end

function Codegen:generateDeclaration(node)
    local varOffset = self:allocateLocalVar(node.name)
    
    if node.initialValue then
        -- Generate code for the initial value and store it
        self:generateNode(node.initialValue)
        self:emit("    mov dword [rbp" .. varOffset .. "], eax")
    else
        -- Initialize to 0 by default
        self:emit("    mov dword [rbp" .. varOffset .. "], 0")
    end
end

function Codegen:generateAssignment(node)
    -- First evaluate the right side
    self:generateNode(node.right)
    
    -- If the left side is an identifier, store to the variable
    if node.left.type == AST.NODE_TYPES.IDENTIFIER then
        local varOffset = self:getLocalVarOffset(node.left.name)
        if varOffset then
            self:emit("    mov dword [rbp" .. varOffset .. "], eax")
        else
            error("Undefined variable: " .. node.left.name)
        end
    else
        error("Unsupported left-hand side in assignment")
    end
end

function Codegen:generateReturn(node)
    if node.value then
        self:generateNode(node.value)
    else
        self:emit("    mov rax, 0")
    end
    
    self:emit("    mov rsp, rbp")
    self:emit("    pop rbp")
    self:emit("    ret")
end

function Codegen:generateIf(node)
    local labelElse = "if_else_" .. self:genLabelId()
    local labelEnd = "if_end_" .. self:genLabelId()
    
    -- Generate condition code
    self:generateNode(node.condition)
    self:emit("    cmp eax, 0")
    
    if node.elseBlock then
        self:emit("    je " .. labelElse)
    else
        self:emit("    je " .. labelEnd)
    end
    
    -- Generate then block
    self:generateNode(node.thenBlock)
    
    if node.elseBlock then
        self:emit("    jmp " .. labelEnd)
        self:emit(labelElse .. ":")
        self:generateNode(node.elseBlock)
    end
    
    self:emit(labelEnd .. ":")
end

function Codegen:generateWhile(node)
    local labelStart = "while_start_" .. self:genLabelId()
    local labelEnd = "while_end_" .. self:genLabelId()
    
    self:emit(labelStart .. ":")
    
    -- Generate condition code
    self:generateNode(node.condition)
    self:emit("    cmp eax, 0")
    self:emit("    je " .. labelEnd)
    
    -- Generate loop body
    self:generateNode(node.body)
    
    self:emit("    jmp " .. labelStart)
    self:emit(labelEnd .. ":")
end

function Codegen:generateFor(node)
    local labelStart = "for_start_" .. self:genLabelId()
    local labelEnd = "for_end_" .. self:genLabelId()
    
    -- Initialize
    if node.init then
        self:generateNode(node.init)
    end
    
    self:emit(labelStart .. ":")
    
    -- Check condition
    if node.condition then
        self:generateNode(node.condition)
        self:emit("    cmp eax, 0")
        self:emit("    je " .. labelEnd)
    end
    
    -- Generate loop body
    self:generateNode(node.body)
    
    -- Update
    if node.update then
        self:generateNode(node.update)
    end
    
    self:emit("    jmp " .. labelStart)
    self:emit(labelEnd .. ":")
end

function Codegen:generateBinaryExpr(node)
    if node.operator == ".." then
        -- String concatenation operator
        return self:generateStringConcat(node)
    end
    
    -- Push registers to preserve right value
    self:emit("    push rax")
    
    -- Generate right side
    self:generateNode(node.right)
    self:emit("    push rax")
    
    -- Generate left side
    self:generateNode(node.left)
    self:emit("    pop rbx")
    
    if node.operator == "+" then
        self:emit("    add rax, rbx")
    elseif node.operator == "-" then
        self:emit("    sub rax, rbx")
    elseif node.operator == "*" then
        self:emit("    imul rax, rbx")
    elseif node.operator == "/" then
        self:emit("    xor rdx, rdx") -- Clear rdx for division
        self:emit("    div rbx")
    elseif node.operator == "==" then
        self:emit("    cmp rax, rbx")
        self:emit("    sete al")
        self:emit("    movzx rax, al")
    elseif node.operator == "!=" then
        self:emit("    cmp rax, rbx")
        self:emit("    setne al")
        self:emit("    movzx rax, al")
    elseif node.operator == "<" then
        self:emit("    cmp rax, rbx")
        self:emit("    setl al")
        self:emit("    movzx rax, al")
    elseif node.operator == ">" then
        self:emit("    cmp rax, rbx")
        self:emit("    setg al")
        self:emit("    movzx rax, al")
    elseif node.operator == "<=" then
        self:emit("    cmp rax, rbx")
        self:emit("    setle al")
        self:emit("    movzx rax, al")
    elseif node.operator == ">=" then
        self:emit("    cmp rax, rbx")
        self:emit("    setge al")
        self:emit("    movzx rax, al")
    elseif node.operator == "&&" then
        local labelFalse = "logical_and_false_" .. self:genLabelId()
        local labelEnd = "logical_and_end_" .. self:genLabelId()
        self:emit("    cmp rax, 0")
        self:emit("    je " .. labelFalse)
        self:emit("    cmp rbx, 0")
        self:emit("    je " .. labelFalse)
        self:emit("    mov rax, 1")
        self:emit("    jmp " .. labelEnd)
        self:emit(labelFalse .. ":")
        self:emit("    mov rax, 0")
        self:emit(labelEnd .. ":")
    elseif node.operator == "||" then
        local labelTrue = "logical_or_true_" .. self:genLabelId()
        local labelEnd = "logical_or_end_" .. self:genLabelId()
        self:emit("    cmp rax, 0")
        self:emit("    jne " .. labelTrue)
        self:emit("    cmp rbx, 0")
        self:emit("    jne " .. labelTrue)
        self:emit("    mov rax, 0")
        self:emit("    jmp " .. labelEnd)
        self:emit(labelTrue .. ":")
        self:emit("    mov rax, 1")
        self:emit(labelEnd .. ":")
    else
        error("Unsupported binary operator: " .. node.operator)
    end
    
    -- Restore registers
    self:emit("    pop rbx")
end

function Codegen:generateStringConcat(node)
    -- Generate the right operand first and push onto stack
    self:generateNode(node.right)
    self:emit("    push rax")
    
    -- Generate the left operand 
    self:generateNode(node.left)
    self:emit("    pop rbx")
    
    -- Now rax contains left string, rbx contains right string
    -- Calculate string lengths
    self:emit("    push rax")
    self:emit("    mov rdi, rax")
    self:emit("    call strlen")
    self:emit("    mov r12, rax")  -- r12 = length of left string
    
    self:emit("    mov rdi, rbx")
    self:emit("    call strlen")
    self:emit("    mov r13, rax")  -- r13 = length of right string
    
    -- Calculate total length needed for new string
    self:emit("    add r13, r12")
    self:emit("    inc r13")  -- +1 for null terminator
    
    -- Allocate memory for new string
    self:emit("    push rbx")
    self:emit("    mov rdi, r13")
    self:emit("    call malloc")
    self:emit("    mov r14, rax")  -- r14 = new string address
    
    -- Copy strings (using rep movsb)
    self:emit("    mov rdi, rax")
    self:emit("    pop rbx")
    self:emit("    pop rax")
    self:emit("    mov rsi, rax")
    self:emit("    mov rcx, r12")
    self:emit("    rep movsb")
    
    self:emit("    mov rsi, rbx")
    self:emit("    mov rcx, r13")
    self:emit("    sub rcx, r12")
    self:emit("    rep movsb")
    
    -- Add null terminator
    self:emit("    mov byte [rdi], 0")
    
    -- Result is in r14
    self:emit("    mov rax, r14")
end

function Codegen:generateUnaryExpr(node)
    self:generateNode(node.operand)
    
    if node.operator == "-" then
        self:emit("    neg rax")
    elseif node.operator == "!" then
        self:emit("    cmp rax, 0")
        self:emit("    sete al")
        self:emit("    movzx rax, al")
    else
        error("Unsupported unary operator: " .. node.operator)
    end
end

function Codegen:generateLiteral(node)
    self:emit("    mov dword eax, " .. node.value)
end

function Codegen:generateStringLiteral(node)
    local id = self:addStringLiteral(node.value)
    self:emit("section .data")
    self:emit("    str_" .. id .. ": db " .. self:stringToBytes(node.value) .. ", 0")
    self:emit("section .text")
    self:emit("    lea rax, [str_" .. id .. "]")
end

function Codegen:generateIdentifier(node)
    local offset = self:getLocalVarOffset(node.name)
    if offset then
        self:emit("    movsxd rax, dword [rbp" .. offset .. "]")
    else
        error("Undefined variable: " .. node.name)
    end
end

function Codegen:generateFunctionCall(node)
    -- Handle built-in functions
    if node.name == "write" then
        if #node.args ~= 1 then
            error("write() requires exactly one argument")
        end
        
        -- Generate code for the argument
        self:generateNode(node.args[1])
        -- The string pointer is now in rax
        self:emit("    mov rsi, rax")
        
        -- Allocate buffer space on stack
        self:emit("    sub rsp, 32")
        self:emit("    mov rdi, rsp")
        
        -- Use a helper function to create printable string
        if node.args[1].type == AST.NODE_TYPES.IDENTIFIER or
           node.args[1].type == AST.NODE_TYPES.LITERAL then
            -- Convert integer to string
            self:emit("    call int_to_string")
        end
        
        -- Write to stdout
        self:emit("    mov rdx, rax")  -- string length
        self:emit("    mov rax, 1")    -- sys_write
        self:emit("    mov rdi, 1")    -- stdout
        self:emit("    syscall")
        
        self:emit("    add rsp, 32")
        return
    end
    
    -- Regular function call
    -- Push arguments in reverse order
    for i = #node.args, 1, -1 do
        self:generateNode(node.args[i])
        self:emit("    push rax")
    end
    
    -- Call the function
    self:emit("    call " .. node.name)
    
    -- Clean up the stack
    if #node.args > 0 then
        self:emit("    add rsp, " .. (8 * #node.args))
    end
end

-- Helper methods
function Codegen:allocateLocalVar(name)
    self.localVarCount = self.localVarCount + 1
    local offset = -4 * self.localVarCount
    self.localVars[name] = offset
    return offset >= 0 and "+" .. offset or tostring(offset)
end

function Codegen:getLocalVarOffset(name)
    local offset = self.localVars[name]
    return offset and (offset >= 0 and "+" .. offset or tostring(offset))
end

function Codegen:genLabelId()
    if not self.labelCounter then
        self.labelCounter = 0
    end
    self.labelCounter = self.labelCounter + 1
    return self.labelCounter
end

function Codegen:addStringLiteral(str)
    self.stringLiteralCount = self.stringLiteralCount + 1
    self.stringLiterals[self.stringLiteralCount] = str
    return self.stringLiteralCount
end

function Codegen:stringToBytes(str)
    local bytes = {}
    for i = 1, #str do
        local byte = str:byte(i)
        if byte >= 32 and byte <= 126 then  -- printable ASCII
            bytes[i] = "'" .. string.char(byte) .. "'"
        else
            bytes[i] = tostring(byte)
        end
    end
    return table.concat(bytes, ",")
end

return Codegen