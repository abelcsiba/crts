" Define the syntax for mylang
syntax clear

" Keywords
syntax keyword mylangKeyword entry var loop if else print read return import module pure proc record entity true false as

" Types
syntax keyword mylangType i8 i16 i32 i64 string bool char float double null void

" Strings
syntax region mylangString start=+"+ end=+"+

syntax match mylangChar "'[^']'"

syntax match mylangFunction "\<pure\>\s\+\w\+\s*(.*)\s*->"

syntax match mylangVariable "\<var\s\+\w\+\>"

" Numbers
syntax match mylangNumber "\<\d\+\>"

" Comments (assuming single-line comments start with //)
syntax match mylangComment "//.*$"

" Define highlighting
highlight link mylangKeyword Keyword
highlight link mylangType Type
highlight link mylangString String
highlight link mylangNumber Number
highlight link mylangComment Comment
highlight link mylangChar Character
highlight link mylangFunction Function
highlight link mylangVariable Identifier
