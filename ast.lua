local AST = {}
AST.__index = AST

AST.NODE_TYPES = {
    -- Program structure
    PROGRAM = "PROGRAM",
    FUNCTION_DECL = "FUNCTION_DECL",
    BLOCK = "BLOCK",
    
    -- Declarations
    DECLARATION = "DECLARATION",
    STRUCT_DECL = "STRUCT_DECL",
    UNION_DECL = "UNION_DECL",
    TYPEDEF_DECL = "TYPEDEF_DECL",
    
    -- Statements
    ASSIGNMENT = "ASSIGNMENT",
    RETURN_STMT = "RETURN_STMT",
    IF_STMT = "IF_STMT",
    WHILE_STMT = "WHILE_STMT",
    DO_WHILE_STMT = "DO_WHILE_STMT",
    FOR_STMT = "FOR_STMT",
    SWITCH_STMT = "SWITCH_STMT",
    CASE_STMT = "CASE_STMT",
    BREAK_STMT = "BREAK_STMT",
    CONTINUE_STMT = "CONTINUE_STMT",
    
    -- Expressions
    BINARY_EXPR = "BINARY_EXPR",
    UNARY_EXPR = "UNARY_EXPR",
    LITERAL = "LITERAL",
    STRING_LITERAL = "STRING_LITERAL",
    IDENTIFIER = "IDENTIFIER",
    FUNCTION_CALL = "FUNCTION_CALL",
    ARRAY_ACCESS = "ARRAY_ACCESS",
    STRUCT_ACCESS = "STRUCT_ACCESS",
    POINTER_ACCESS = "POINTER_ACCESS",
    SIZEOF_EXPR = "SIZEOF_EXPR"
}

function AST.createNode(type, attrs)
    local node = { type = type }
    if attrs then
        for k, v in pairs(attrs) do
            node[k] = v
        end
    end
    return node
end

function AST.visit(node, visitor)
    if not node then return end
    
    local enterFunc = visitor[node.type .. "_enter"]
    if enterFunc then enterFunc(node) end
    
    -- Visit children based on node type
    if node.type == AST.NODE_TYPES.PROGRAM then
        for _, stmt in ipairs(node.statements) do
            AST.visit(stmt, visitor)
        end
    elseif node.type == AST.NODE_TYPES.FUNCTION_DECL then
        for _, param in ipairs(node.params) do
            AST.visit(param, visitor)
        end
        AST.visit(node.body, visitor)
    elseif node.type == AST.NODE_TYPES.BLOCK then
        for _, stmt in ipairs(node.statements) do
            AST.visit(stmt, visitor)
        end
    elseif node.type == AST.NODE_TYPES.DECLARATION then
        if node.initialValue then
            AST.visit(node.initialValue, visitor)
        end
    elseif node.type == AST.NODE_TYPES.ASSIGNMENT then
        AST.visit(node.left, visitor)
        AST.visit(node.right, visitor)
    elseif node.type == AST.NODE_TYPES.RETURN_STMT then
        if node.value then
            AST.visit(node.value, visitor)
        end
    elseif node.type == AST.NODE_TYPES.IF_STMT then
        AST.visit(node.condition, visitor)
        AST.visit(node.thenBlock, visitor)
        if node.elseBlock then
            AST.visit(node.elseBlock, visitor)
        end
    elseif node.type == AST.NODE_TYPES.WHILE_STMT then
        AST.visit(node.condition, visitor)
        AST.visit(node.body, visitor)
    elseif node.type == AST.NODE_TYPES.FOR_STMT then
        if node.init then AST.visit(node.init, visitor) end
        if node.condition then AST.visit(node.condition, visitor) end
        if node.update then AST.visit(node.update, visitor) end
        AST.visit(node.body, visitor)
    elseif node.type == AST.NODE_TYPES.BINARY_EXPR then
        AST.visit(node.left, visitor)
        AST.visit(node.right, visitor)
    elseif node.type == AST.NODE_TYPES.UNARY_EXPR then
        AST.visit(node.operand, visitor)
    elseif node.type == AST.NODE_TYPES.FUNCTION_CALL then
        for _, arg in ipairs(node.args) do
            AST.visit(arg, visitor)
        end
    end
    
    local exitFunc = visitor[node.type .. "_exit"]
    if exitFunc then exitFunc(node) end
    
    local visitFunc = visitor[node.type]
    if visitFunc then visitFunc(node) end
end

return AST 