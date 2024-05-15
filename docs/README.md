# Project Phase 1

## LOAD

### Logic

1. For Loading a Matrix, we have created a function **updateMatrixInfo** inside Matrix class which will be used for updating the metadata namely **columnLength** *(keep tracks of number of columns in a matrix)*, **subMatrixDim** *(maximum row/column stored inside a block)*.
2. If we are successfully able to store the metadata, then we start reading the file line by line and keep updating the **rowCount** *(keeps track of total number of rows in a matrix)* of Matrix class.
3. We keep storing entire rows in a 2d vector and whenever we get rows equal to **subMatrixDim**, we call **addBlocks** function which will iteratively create sub-blocks equal to data that can fit in one page and then uses **writeMatrixPage** of **BufferManager** class to make pages of this sub-blocks and store it in a file.
4. Calculation of total sub-blocks
    - subMatrixDim = ceil((BLOCK_SIZE * 1000) / n)
    - blocksPerRow = ceil(n / subMatrixDim)
    - **total_sub_blocks** = (blocksPerRow * blockPerRow)

#### Number of Blocks Read: 0
#### Number of Blocks Write: total_sub_blocks
#### Number of Blocks Accessed: total_sub_blocks


## PRINT

### Logic

1. First we calculate the number of rows and columns we need to print on console, based on that we calculate the blocks we need to access to accomodate this many rows and columns.
2. We used a logic similar to that of load, i.e for each rowBlock we fetch all its column blocks using cursor page by page using method **getRowBlocksData** defined in Cursor class.
3. For each rowBlock, all the blocks fetched using above steps are then printed onto the console using **writeRowBlockData** function defined in Matrix class.

#### Number of Blocks Read: total_sub_blocks
#### Number of Blocks Write: 0
#### Number of Blocks Accessed: total_sub_blocks 

## TRANSPOSE

### Logic

1. For transposing a matrix, we calculated the indexes of the two blocks that would be swapped with each other.
2. Then with the help of cursor we have fetched those two blocks and stored each of them in a 2d vector.
3. Transposed both the 2d vectors with one another and wrote those two pages back to the files in which corresponding pages are stored.
4. After step 3, still if the page is present in pool it would not be the page which has been transposed so we removed that page from the pool using **removePage** defined in BufferManager class, so whenever next time this page is needed we will fetch it from the secondary memory where updated transpose of a matrix block is present.

#### Number of Blocks Read: total_sub_blocks
#### Number of Blocks Write: total_sub_blocks
#### Number of Blocks Accessed: 2 * total_sub_blocks

## COMPUTE

### Logic

1. Logic for compute command is similar to that of transpose command, here also we fetch two blocks corresponding to the index we calculate say those blocks as **matBlock1** and **matBlock2Transpose**
2. **matBlock2Transpose** will contain transpose of the block corresponding to the **matBlock1**.
3. We then subtract cell by cell of **matBlock2Transpose** from **matBlock1**.
4. The above operation is performed for each rowBlock, and after each rowBlock is processed we keep adding result into the **MATRIXNAME_RESULT.csv** file.

#### Number of Blocks Read: 2 * total_sub_blocks
#### Number of Blocks Write: total_sub_blocks
#### Number of Blocks Accessed: 3 * total_sub_blocks

## CHECKSYMMETRY

### Logic

1. For this command two corresponding blockIndexes are calculated, and with help of cursor those two blocks are fetched.
2. We then compare both the blocks if at any point they are not equal, it means matrix is not symmetric so we return false.
3. If we are successfully able to check for all the matrix blocks, it implies that matrix is symmetric so we return true.

#### Number of Blocks Read: total_sub_blocks
#### Number of Blocks Write: 0
#### Number of Blocks Accessed: total_sub_blocks 

## EXPORT

### Logic

1. For this command we fetch all the blocks using cursor present for each rowBlock index and keep storing them in a 2d vector.
2. Once a rowBlock is processed, we append the 2d vector in a file named **MATRIXNAME_EXPORT.csv**.
3. We keep repeating the above process till all the blocks of a matrix are processed.

#### Number of Blocks Read: total_sub_blocks
#### Number of Blocks Write: 0
#### Number of Blocks Accessed: total_sub_blocks 


## RENAME

### Logic

1. First in semantic parser we have checked that does the mentioned matrix exist in matrix catalogue,
   if yes then we have check that does the matrix with new name already exist if no then only renaming will happen otherwise error will be generated.
2. Renaming is done in two steps
    - Renaming the matrix name in matrix object which is present in matrix catalogue.
    - Renaming all the pages of with new matrix name.

#### Number of Blocks Read: 0
#### Number of Blocks Write: 0
#### Number of Blocks Accessed: 0


## Assumptions

1. For PRINT command there is a trade-off between memory usage and block access.
    - Let's say after dividing the matrix in sub-matrices(blocks) the new matrix formed of blocks is having dimention as d*d.
    - **Method-1:**
        - Read all the 'd' blocks of one row and store it into the 2d-vector which will be stored in main-memory and from that vector print the rows that we need.
        - If rows are more than each sub-block then do same process for the next row, and go until all the required rows are not fetched i.e min(PRINT_COUNT, n). Here, memory is used more but block access are less.
    - **Method-2:**
        - Fetch only one row at a time i.e read 'd' blocks for printing every row. Here, the total block access will increase i.e. min(PRINT_COUNT, n) * d.
    - We have used method-1 in our code considering the fact of reducing the block access and not memory usage.
2. For the compute command we are storing the result **MATRIXNAME_RESULT** in a non-csv file, the file **MATRIXNAME_RESULT** is stored in form of multiple pages.
3. Number of Blocks Read, Write and Accessed mentioned in above commands denotes the upper limit.
4. Appropriate error checking using syntactic and semantic parser has been done for all the commands. 

## Learnings

1. Understanding the code base.
2. How different strategies of storing data can increase/decrease the number of block access and memory usages.
3. Working with the given constraints of loading pages/blocks in main memory at a same time.
4. Thinking of the best strategy for optimizing the block access count for given memory usage constraints.

## Work Distribution

### Meet Patel (2022201002)
1. PRINT
2. TRANSPOSE
3. CHECKSYMMETRY
4. Syntactic Parser
5. REPORT - Learnings

### Jeet Shah (2022201009)
1. LOAD
2. COMPUTE
3. EXPORT
4. Semantic Parser
5. REPORT - Assumptions

### Praddyumn Shukla (2022201001)
1. REPORT - Commands desc
2. RENAME
