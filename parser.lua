local AST = require("ast")

local Parser = {}
Parser.__index = Parser

function Parser.new(lexer)
    local self = setmetatable({}, Parser)
    self.lexer = lexer
    self.currentToken = nil
    self.nextToken = nil
    self:advance()
    self:advance()
    return self
end

function Parser:advance()
    self.currentToken = self.nextToken
    self.nextToken = self.lexer:nextToken()
end

function Parser:expect(type, value)
    if self.currentToken.type ~= type or (value and self.currentToken.value ~= value) then
        error("Expected " .. type .. (value and " " .. value or "") .. ", got " .. self.currentToken.type)
    end
    self:advance()
end

function Parser:parse()
    local statements = {}
    while self.currentToken.type ~= "EOF" do
        table.insert(statements, self:parseStatement())
    end
    return AST.createNode(AST.NODE_TYPES.PROGRAM, { statements = statements })
end

function Parser:parseStatement()
    if self.currentToken.type == "KEYWORD" then
        if self.currentToken.value == "int" then
            self:advance()  -- consume 'int'
            local name = self.currentToken.value
            self:advance()  -- consume identifier
            
            -- Check if this is a function declaration
            if self.currentToken.type == "SEPARATOR" and self.currentToken.value == "(" then
                self:advance()  -- consume '('
                self:expect("SEPARATOR", ")")
                local body = self:parseBlock()
                return AST.createNode(AST.NODE_TYPES.FUNCTION_DECL, {
                    name = name,
                    body = body
                })
            else
                -- Variable declaration
                local initialValue = nil
                if self.currentToken.type == "OPERATOR" and self.currentToken.value == "=" then
                    self:advance()  -- consume '='
                    initialValue = self:parseExpression()
                end
                self:expect("SEPARATOR", ";")
                return AST.createNode(AST.NODE_TYPES.DECLARATION, {
                    name = name,
                    initialValue = initialValue
                })
            end
        elseif self.currentToken.value == "return" then
            return self:parseReturn()
        elseif self.currentToken.value == "if" then
            return self:parseIf()
        elseif self.currentToken.value == "while" then
            return self:parseWhile()
        elseif self.currentToken.value == "for" then
            return self:parseFor()
        end
    end
    
    local expr = self:parseExpression()
    self:expect("SEPARATOR", ";")
    return expr
end

function Parser:parseFunctionDecl()
    self:expect("KEYWORD", "int")
    local name = self.currentToken.value
    self:expect("IDENTIFIER")
    self:expect("SEPARATOR", "(")
    self:expect("SEPARATOR", ")")
    
    local body = self:parseBlock()
    return AST.createNode(AST.NODE_TYPES.FUNCTION_DECL, {
        name = name,
        body = body
    })
end

function Parser:parseDeclaration()
    self:expect("KEYWORD", "int")
    local name = self.currentToken.value
    self:expect("IDENTIFIER")
    
    local initialValue = nil
    if self.currentToken.type == "OPERATOR" and self.currentToken.value == "=" then
        self:advance()
        initialValue = self:parseExpression()
    end
    
    self:expect("SEPARATOR", ";")
    return AST.createNode(AST.NODE_TYPES.DECLARATION, {
        name = name,
        initialValue = initialValue
    })
end

function Parser:parseReturn()
    self:expect("KEYWORD", "return")
    local value = self:parseExpression()
    if not value then
        error("Return statement must have a value")
    end
    self:expect("SEPARATOR", ";")
    return AST.createNode(AST.NODE_TYPES.RETURN_STMT, { value = value })
end

function Parser:parseIf()
    self:expect("KEYWORD", "if")
    self:expect("SEPARATOR", "(")
    local condition = self:parseExpression()
    self:expect("SEPARATOR", ")")
    
    local thenBlock = self:parseBlock()
    local elseBlock = nil
    
    if self.currentToken.type == "KEYWORD" and self.currentToken.value == "else" then
        self:advance()
        elseBlock = self:parseBlock()
    end
    
    return AST.createNode(AST.NODE_TYPES.IF_STMT, {
        condition = condition,
        thenBlock = thenBlock,
        elseBlock = elseBlock
    })
end

function Parser:parseWhile()
    self:expect("KEYWORD", "while")
    self:expect("SEPARATOR", "(")
    local condition = self:parseExpression()
    self:expect("SEPARATOR", ")")
    
    local body = self:parseBlock()
    return AST.createNode(AST.NODE_TYPES.WHILE_STMT, {
        condition = condition,
        body = body
    })
end

function Parser:parseFor()
    self:expect("KEYWORD", "for")
    self:expect("SEPARATOR", "(")
    
    local init = nil
    if self.currentToken.type ~= "SEPARATOR" or self.currentToken.value ~= ";" then
        init = self:parseStatement()
    else
        self:advance()
    end
    
    local condition = nil
    if self.currentToken.type ~= "SEPARATOR" or self.currentToken.value ~= ";" then
        condition = self:parseExpression()
    end
    self:expect("SEPARATOR", ";")
    
    local update = nil
    if self.currentToken.type ~= "SEPARATOR" or self.currentToken.value ~= ")" then
        update = self:parseExpression()
    end
    self:expect("SEPARATOR", ")")
    
    local body = self:parseBlock()
    return AST.createNode(AST.NODE_TYPES.FOR_STMT, {
        init = init,
        condition = condition,
        update = update,
        body = body
    })
end

function Parser:parseBlock()
    self:expect("SEPARATOR", "{")
    local statements = {}
    while self.currentToken.type ~= "SEPARATOR" or self.currentToken.value ~= "}" do
        table.insert(statements, self:parseStatement())
    end
    self:expect("SEPARATOR", "}")
    return AST.createNode(AST.NODE_TYPES.BLOCK, { statements = statements })
end

function Parser:parseExpression()
    return self:parseAssignment()
end

function Parser:parseAssignment()
    local left = self:parseAdditive()
    if self.currentToken.type == "OPERATOR" and self.currentToken.value == "=" then
        self:advance()
        local right = self:parseAssignment()
        return AST.createNode(AST.NODE_TYPES.ASSIGNMENT, {
            left = left,
            right = right
        })
    end
    return left
end

function Parser:parseAdditive()
    local left = self:parseMultiplicative()
    while self.currentToken.type == "OPERATOR" and 
          (self.currentToken.value == "+" or 
           self.currentToken.value == "-" or 
           self.currentToken.value == "..") do
        local op = self.currentToken.value
        self:advance()
        local right = self:parseMultiplicative()
        left = AST.createNode(AST.NODE_TYPES.BINARY_EXPR, {
            operator = op,
            left = left,
            right = right
        })
    end
    return left
end

function Parser:parseMultiplicative()
    local left = self:parsePrimary()
    while self.currentToken.type == "OPERATOR" and (self.currentToken.value == "*" or self.currentToken.value == "/") do
        local op = self.currentToken.value
        self:advance()
        local right = self:parsePrimary()
        left = AST.createNode(AST.NODE_TYPES.BINARY_EXPR, {
            operator = op,
            left = left,
            right = right
        })
    end
    return left
end

function Parser:parsePrimary()
    if self.currentToken.type == "NUMBER" then
        local value = self.currentToken.value
        self:advance()
        return AST.createNode(AST.NODE_TYPES.LITERAL, { value = value })
    elseif self.currentToken.type == "STRING" then
        local value = self.currentToken.value
        self:advance()
        return AST.createNode(AST.NODE_TYPES.STRING_LITERAL, { value = value })
    elseif self.currentToken.type == "IDENTIFIER" then
        local name = self.currentToken.value
        self:advance()
        if self.currentToken.type == "SEPARATOR" and self.currentToken.value == "(" then
            return self:parseFunctionCall(name)
        end
        return AST.createNode(AST.NODE_TYPES.IDENTIFIER, { name = name })
    elseif self.currentToken.type == "SEPARATOR" and self.currentToken.value == "(" then
        self:advance()
        local expr = self:parseExpression()
        self:expect("SEPARATOR", ")")
        return expr
    end
    error("Unexpected token: " .. self.currentToken.type)
end

function Parser:parseFunctionCall(name)
    self:expect("SEPARATOR", "(")
    local args = {}
    if self.currentToken.type ~= "SEPARATOR" or self.currentToken.value ~= ")" then
        table.insert(args, self:parseExpression())
        while self.currentToken.type == "SEPARATOR" and self.currentToken.value == "," do
            self:advance()
            table.insert(args, self:parseExpression())
        end
    end
    self:expect("SEPARATOR", ")")
    return AST.createNode(AST.NODE_TYPES.FUNCTION_CALL, {
        name = name,
        args = args
    })
end

return Parser 