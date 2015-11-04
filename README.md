# zaxbase

## Aim

implement a MiniSQL engine


## Requirement

### supported datatype:
- int 
- char(n) (fixed length)
- float

optional:

- varchar

### table definition

- supported maximum 32 attributes
- support unique
- support simple primary key

### index create and delete

- auto create B+ tree index for primary key
- use sql command to create index for unique attributes

### search records

- support `and` `=` `range query`

### insert and delete record

- support to insert and delete multiple records


## Work

### xyw
- Interpreter
- API
- Buffer Manager

### zax
- Record Manager
- Index Manager

### dqm
- Catalog Manager
