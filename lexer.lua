local Lexer = {}
Lexer.__index = Lexer

Lexer.TOKEN_TYPES = {
    KEYWORD = "KEYWORD",
    IDENTIFIER = "IDENTIFIER",
    NUMBER = "NUMBER",
    STRING = "STRING",
    OPERATOR = "OPERATOR",
    SEPARATOR = "SEPARATOR",
    EOF = "EOF"
}

Lexer.KEYWORDS = {
    ["int"] = true,
    ["return"] = true,
    ["if"] = true,
    ["else"] = true,
    ["while"] = true,
    ["for"] = true,
    ["char"] = true,
    ["float"] = true,
    ["double"] = true,
    ["void"] = true,
    ["do"] = true,
    ["break"] = true,
    ["continue"] = true,
    ["switch"] = true,
    ["case"] = true,
    ["default"] = true,
    ["struct"] = true,
    ["union"] = true,
    ["typedef"] = true,
    ["const"] = true,
    ["sizeof"] = true
}

Lexer.OPERATORS = {
    ["+"] = true, ["-"] = true, ["*"] = true, ["/"] = true,
    ["="] = true,
    ["=="] = true, ["!="] = true,
    ["<"] = true, [">"] = true,
    ["<="] = true, [">="] = true,
    ["&&"] = true, ["||"] = true,
    ["!"] = true,
    ["++"] = true, ["--"] = true,
    ["+="] = true, ["-="] = true,
    ["*="] = true, ["/="] = true,
    ["&"] = true, ["|"] = true,
    ["^"] = true, ["~"] = true,
    ["<<"] = true, [">>"] = true,
    [".."] = true  -- String concatenation operator
}

Lexer.SEPARATORS = {
    ["("] = true, [")"] = true, ["{"] = true, ["}"] = true,
    ["["] = true, ["]"] = true, [";"] = true, [","] = true
}

function Lexer.new(source, filename)
    return setmetatable({
        source = source,
        filename = filename or "unknown",
        tokens = {},
        line = 1,
        column = 1,
        pos = 1
    }, Lexer)
end

function Lexer:createToken(type, value)
    return {
        type = type,
        value = value,
        line = self.line,
        column = self.column,
        filename = self.filename
    }
end

function Lexer:advance(n)
    n = n or 1
    self.pos = self.pos + n
    self.column = self.column + n
end

function Lexer:peek(offset)
    return self.source:sub(self.pos + (offset or 0), self.pos + (offset or 0))
end

function Lexer:tokenize()
    while self.pos <= #self.source do
        local char = self:peek()
        
        if char == "\n" then
            self.line = self.line + 1
            self.column = 1
            self:advance()
        elseif char:match("%s") then
            self:advance()
        elseif char:match("%d") then
            self:parseNumber()
        elseif char:match("[%a_]") then
            self:parseIdentifierOrKeyword()
        elseif char == '"' then
            self:parseString()
        elseif char == "/" and self:peek(1) == "/" then
            self:skipComment()
        elseif self.OPERATORS[char] or self.OPERATORS[char .. self:peek(1)] then
            self:parseOperator()
        elseif self.SEPARATORS[char] then
            table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.SEPARATOR, char))
            self:advance()
        else
            self:advance()
        end
    end
    
    table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.EOF, "EOF"))
    return self.tokens
end

function Lexer:parseNumber()
    local num = ""
    while self.pos <= #self.source and self:peek():match("%d") do
        num = num .. self:peek()
        self:advance()
    end
    table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.NUMBER, num))
end

function Lexer:parseIdentifierOrKeyword()
    local id = ""
    while self.pos <= #self.source and self:peek():match("[%a%d_]") do
        id = id .. self:peek()
        self:advance()
    end
    
    local type = self.KEYWORDS[id] and self.TOKEN_TYPES.KEYWORD or self.TOKEN_TYPES.IDENTIFIER
    table.insert(self.tokens, self:createToken(type, id))
end

function Lexer:parseOperator()
    local op = self:peek()
    local nextChar = self:peek(1)
    local twoCharOp = op .. nextChar
    
    -- Check for two-character operators first
    if self.OPERATORS[twoCharOp] then
        table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.OPERATOR, twoCharOp))
        self:advance(2)
    else
        table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.OPERATOR, op))
        self:advance()
    end
end

function Lexer:skipComment()
    while self.pos <= #self.source and self:peek() ~= "\n" do
        self:advance()
    end
end

function Lexer:parseString()
    self:advance()  -- Skip opening quote
    local str = ""
    while self.pos <= #self.source and self:peek() ~= '"' do
        if self:peek() == "\\" then
            self:advance()
            local escaped = self:peek()
            if escaped == "n" then
                str = str .. "\n"
            elseif escaped == "t" then
                str = str .. "\t"
            elseif escaped == "r" then
                str = str .. "\r"
            else
                str = str .. escaped
            end
        else
            str = str .. self:peek()
        end
        self:advance()
    end
    
    if self:peek() ~= '"' then
        error("Unterminated string literal")
    end
    self:advance()  -- Skip closing quote
    
    table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.STRING, str))
end

function Lexer:parseCharacter()
    self:advance() -- Skip opening quote
    
    local value = ""
    if self:peek() == "\\" then
        self:advance()
        local escaped = self:peek()
        if escaped == "n" then
            value = "\n"
        elseif escaped == "t" then
            value = "\t"
        elseif escaped == "r" then
            value = "\r"
        elseif escaped == "\\" then
            value = "\\"
        elseif escaped == "'" then
            value = "'"
        elseif escaped == "\"" then
            value = "\""
        elseif escaped == "0" then
            value = "\0"
        elseif escaped:match("%d") then
            -- Octal escape
            local octal = escaped
            self:advance()
            for i = 1, 2 do
                if self.pos <= #self.source and self:peek():match("[0-7]") then
                    octal = octal .. self:peek()
                    self:advance()
                else
                    break
                end
            end
            local byte = tonumber(octal, 8)
            value = string.char(byte)
            if self:peek() ~= "'" then
                self:error("Expected closing quote after character literal")
            end
            self:advance() -- Skip closing quote
            table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.NUMBER, tostring(byte)))
            return
        elseif escaped == "x" then
            -- Hex escape
            self:advance() -- Skip 'x'
            local hex = ""
            for i = 1, 2 do
                if self.pos <= #self.source and self:peek():match("[0-9a-fA-F]") then
                    hex = hex .. self:peek()
                    self:advance()
                else
                    self:error("Invalid hex escape sequence")
                    break
                end
            end
            local byte = tonumber(hex, 16)
            value = string.char(byte)
            if self:peek() ~= "'" then
                self:error("Expected closing quote after character literal")
            end
            self:advance() -- Skip closing quote
            table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.NUMBER, tostring(byte)))
            return
        else
            value = escaped
        end
        self:advance()
    else
        value = self:peek()
        self:advance()
    end
    
    if self:peek() ~= "'" then
        self:error("Expected closing quote after character literal")
    else
        self:advance() -- Skip closing quote
    end
    
    table.insert(self.tokens, self:createToken(self.TOKEN_TYPES.NUMBER, tostring(string.byte(value))))
end

-- Get the next token from the stream
function Lexer:nextToken()
    if not self.tokens then
        self:tokenize()
    end
    
    local token = self.tokens[1]
    if token then
        table.remove(self.tokens, 1)
        return token
    end
    
    return self:createToken(self.TOKEN_TYPES.EOF, "EOF")
end

-- Peek at the next token without consuming it
function Lexer:peekToken(offset)
    if not self.tokens then
        self:tokenize()
    end
    
    offset = offset or 0
    if offset + 1 <= #self.tokens then
        return self.tokens[offset + 1]
    end
    
    return self:createToken(self.TOKEN_TYPES.EOF, "EOF")
end

-- Check if there are any errors
function Lexer:hasErrors()
    return #self.errors > 0
end

-- Get all errors
function Lexer:getErrors()
    return self.errors
end

-- Reset the lexer to its initial state
function Lexer:reset()
    self.tokens = {}
    self.line = 1
    self.column = 1
    self.pos = 1
    self.currentToken = nil
    self.errors = {}
end

return Lexer 