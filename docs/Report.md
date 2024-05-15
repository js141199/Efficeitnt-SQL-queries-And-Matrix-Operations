# Project Phase 2

## TASK-1: IMPLEMENTATION OF EXTERNAL SORTING

### Algorithm used for external sorting
**Command**
```
SORT <table_name> BY <column_name1, column_name2,..., column_namek> IN <ASC|DESC, ASC|DESC,..., ASC|DESC>
```
1. For sorting the rows of the table, 2 steps needs to be carried out.
   - Sorting of individual pages/blocks
   - Merging the sorted pages
2. Consider below notations
   - BLOCK_COUNT -> Number of pages that can be loaded at given time in main memory. In this project it is limited to 10
   - BLOCK_SIZE -> Size of each page/block in bytes. In this project it is limited to 1000 bytes.
   - TOTAL_BLOCKS => Total Blocks/pages required to store the specified table.
3. **Step-1:** Sorting of individual pages
   - We will load BLOCK_COUNT number of pages in the main memory and sort the pages based on the given subset of columns and there respected order(i.e. ASC or DESC).
   - By default priority will be considered from left to right column names as mentioned in the query/command.
   - We have implemented one general comparator that will sort the rows of table based on the given columns, order and there priority.
   - We will perform above steps till all the pages of respected table have been sorted internally(i.e. each row of the page is sorted).
4. **Step-2:** Merging operation
   - Once we have sorted all the pages individually we have to now merge all the pages to make whole table sorted.
   - Now, for merging operation we need to load multiple pages in our main memory and then apply k-way sorting (i.e merging of k pages in single iteration).
   - As mentioned above, we can only load BLOCK_COUNT number of pages in our main memory, it means we can only use main memory of at max BLOCK_COUNT * BLOCK_SIZE bytes.
   - So, we can load at max BLOCK_COUNT - 1 blocks in one iteration becuase we need one block memory for storing the resultant rows.
   - We will maintain min(BLOCK_COUNT - 1, Remaining_blocks_for_merging) cursors, each pointing to the different page.
   - We will make use of heap for finding the best match row, that matches our sorting condition from cursors (each pointing to the single row of corresponding page) present in the min-heap.
   - The matched row will be pushed into the block (that we are using storing final results) and whenver the block gets full we will write the page back to the secondary memory and clear the block in main memory.
   - Initially these block will be written inside the temporary block and then converted to the actual block.
   - These merging operation will be performed in multiple levels. Below is the psedocode of each level.
   ```latex
   Calculation of levels needed.
   At level-0 we can merge BLOCK_COUNT - 1 pages in one iteration.
   At level-1 we can merge (BLOCK_COUNT - 1)^2 pages in one iteration.
   .......
   At level-n we can merge (BLOCK_COUNT - 1) ^ (n-1) pages in one iteration.
   So, as the level increases number of pages that will be continously sorted are increasing at the rate of
   (BLOCK_COUNT -1) ^ (level).
   So, total level needed are -> levels = {log_(BLOCK_COUNT-1) TOTAL_BLOCKS }.
   
   for each level from 0 to levels-1:
       pageIndex = 0
      for pageIndex < totalBLocks:
        // at max BLOCK_COUNT - 1 pages can be loaded at any given time
        cursors = BLOCK_COUNT - 1
        list<cursor, totalRowsToBeTaken_starting_from_row_where_cursor_is_initially_pointing_to> cursorList
        // number of blocks that are continously sorted any given level
        mergeBlocksCount = pow(BLOCK_COUNT - 1, level)
        for pageIndex < TOTAL_BLOCKS and cursors > 0:
            ls.add({cursor(pageIndex), totalRows})
            pageIndex += mergeBlocksCount
        Now we will merge all the blocks that we have found in cursorList by adding them into the heap
      Rename all the temporary pages to the actual page names of the table.
   ```
5. To ensure that at any given point to time the number of blocks/pages inside the secondary memory does not exceeds the total number of pages required to store the table, whenever we are loading the page in the main memory we are deleting that page from the secondary memory, so whenever we are writing the temporary pages it will not exceed the total pages needed to store the actual table data.
6. Total block access (Read + Write) needed
   ```
   Step-1: Individually sorting data => (2 * TOTAL_BLOCKS)
   TOTAL_BLOCKS for reading the pages.
   TOTAL_BLOCKS for writing the sorted pages back.

   Step-2: Merging the sorted pages => ({log_(BLOCK_COUNT-1) TOTAL_BLOCKS }) * (2 * TOTAL_BLOCKS)
   {log_(BLOCK_COUNT-1) TOTAL_BLOCKS } -> Number of levels
   (2*TOTAL_BLOCKS) -> TOTAL_BLOCKS for read and TOTAL_BLOCKS for write at each level

   Total access are => (2 * TOTAL_BLOCKS) + ({log_(BLOCK_COUNT-1) TOTAL_BLOCKS } * (2 * TOTAL_BLOCKS))
   ```

## TASK-2: APPLICATIONS OF EXTERNAL SORTING

### JOIN Operation
**Command**
```
<new_relation_name> <- JOIN <tablename1>, <tablename2> ON <column1> <bin_op> <column2>
<bin_op> = "==",">",">=","<","<="
```
1. For '==' operator
   - Firstly the temporary relations from two relations on which we wanted to perform join operations are created and the pages are stored in secondary storage.
   - Both the temporary relations that are created in the above step are sorted using external sort algorithm on their respective columns implemented using Task-1.
   - Sorting the table on the column on which we wanted to perform join helps in reducing the overall time complexity to a certain extent.
   - For each tuple of the first relation we bring the page from the secondary memory using cursor with the help of the map which keeps tracks of a particular key's first and last occurence of the page index in the second relational table.
   - We keep getting the row using cursor pointed to a page on second relational table till the columnKey on first relation is lesser or equal to the columnKey on the second relation.
   - Those tuples which matches (i.e have equal values of columnKey) are being added to a resultantTable which will store our final result of the JOIN operation
   - We stop moving the cursor on second relation as soon as we get a columnKey value of second relation greater than the columnKey value of the first relation, because as the relations are sorted there is no point in checking the further row for the current tuple of the first relation.
   - We simultaneously keep updating the map for the first and the last occurence pageindex of a columnKey.
   - Above steps are repeated for each tuple of the first relation and each time with the help of map we keep pointing the Cursor on the page of the second relation with the help of map we stored using the above steps.
   - The resultant table which we created above is now converted to blocks using existing blockify method, so we can use the PRINT command to print the table in the console.

2. For '<' operator
   - Firstly the temporary relations from two relations on which we wanted to perform join operations are created and the pages are stored in secondary storage.
   - The first relation is now sorted in ascending order and the second relation is sorted in descending order. This type of sorting will help break out of the loop more quickly where the columnKey of the left relation becomes >= columnKey on the right
   - For each tuple of the first relation we bring the page of second relation starting from pageIndex 0
   - We keep getting the row using cursor pointed to a page on second relational table till the columnKey on first relation is lesser or equal to the columnKey on the second relation.
   - Those tuples which matches (i.e have columnKey of first relation < columnKey of second relation) are being added to a resultantTable which will store our final result of the JOIN operation
   - We stop moving the cursor on second relation as soon as we get a columnKey value of second relation greater than or equal to the columnKey value of the first relation, because as the relations are sorted there is no point in checking the further row for the current tuple of the first relation.
   - Above steps are repeated for each tuple of the first relation and we keep pointing the Cursor on the first pageIndex of the second relation and loop from multiple pages till the above condition satisfies.
   - The resultant table which we created above is now converted to blocks using existing blockify method, so we can use the PRINT command to print the table in the console.

3. For '<=' operator
   - Logic for '<=' is same as '<' operator
   - The only thing that changes is the stopping condition, previously we stopped when (columnKey of the left relation becomes >= columnKey on the right)
   - Now our stopping condition will slightly change, it will now be (columnKey of the left relation becomes > columnKey on the right) because we also want those rows which have equal column keys.
   - Rest all the steps are same as the '<' operator.

4. For '>' operator
   - Firstly the temporary relations from two relations on which we wanted to perform join operations are created and the pages are stored in secondary storage.
   - The first relation is now sorted in descending order and the second relation is sorted in ascending order. This type of sorting will help break out of the loop more quickly where the columnKey of the left relation becomes <= columnKey on the right
   - For each tuple of the first relation we bring the page of second relation starting from pageIndex 0
   - We keep getting the row using cursor pointed to a page on second relational table till the columnKey on first relation is lesser or equal to the columnKey on the second relation.
   - Those tuples which matches (i.e have columnKey of first relation > columnKey of second relation) are being added to a resultantTable which will store our final result of the JOIN operation
   - We stop moving the cursor on second relation as soon as we get a columnKey value of second relation less than or equal to the columnKey value of the first relation, because as the relations are sorted there is no point in checking the further row for the current tuple of the first relation.
   - Above steps are repeated for each tuple of the first relation and we keep pointing the Cursor on the first pageIndex of the second relation and loop from multiple pages till the above condition satisfies.
   - The resultant table which we created above is now converted to blocks using existing blockify method, so we can use the PRINT command to print the table in the console.

5. For '>=' operator
   - Logic for '>=' is same as '>' operator
   - The only thing that changes is the stopping condition, previously we stopped when (columnKey of the left relation becomes <= columnKey on the right)
   - Now our stopping condition will slightly change, it will now be (columnKey of the left relation becomes < columnKey on the right) because we also want those rows which have equal column keys.
   - The resultant table which we created above is now converted to blocks using existing blockify method, so we can use the PRINT command to print the table in the console.
   - Rest all the steps are same as the '>' operator.

6. Total block access (Read + Write) in worst case
   ```
   TOTAL_LEFT_RELATION_ROWCOUNT total number of rows in left relation
   TOTAL_RIGHT_BLOCKS total number of blocks in right relation

   Total access are => (TOTAL_LEFT_RELATION_ROWCOUNT * TOTAL_RIGHT_BLOCKS)
   ```


### ORDER BY Operation
**Command**
```
<new_table> <- ORDER BY <attribute> ASC|DESC ON <table_name>
```
1. To perform order by operation, in our first step we have made temporary copies of the pages on which we need to perform operation.
2. We have used the logic of external sort written in task-1 to sort the table based on given column name.
3. Now, we have dumped the resultant sorted pages into the CSV file which will be formed under data/temp folder with Resultant Table relation Name.

### GROUP BY Operation
**Command**
```
<new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING
<aggregate(attribute)> <bin_op> <attribute_value> RETURN
<aggregate_func(attribute)>
```
1. As our first step in the syntactic parsing, we have extracted the aggregate function that are being used for HAVING and RETURN. Apart from this, we also set parameters for certain columns, namely:

- groupByColumn: on which we sort the table as a prerequisite.
- havingColumn: on which the aggregate function for HAVING is being applied.
- returnColumn: the attribute column that we have to return and on which the aggregate function for RETURN is being applied.

2. In the semantic parsing, I just check if the attribute columns and tables in the parsed query are correct or not.

3. Now in the main implementation, we first sort the table in Ascending order on the column mentioned for 'GroupBy'. This is done so that when we are traversing the table row by row in further steps, we can get equal values grouped together in a sorted order.

4. While traversing row by row, we keep track of the last value (prev) that we encountered in the column on which 'GroupBy' is being applied. Whenever we encounter a new value for this attribute, we know that a new Group is starting from here and hence we check the HAVING condition at that step and insert the RETURN attribute value into the Resultant Table if the condition is satisfied.

5. For HAVING, we have broken the problem into two subparts:

- **when aggregate function on HAVING attribute is "MIN" or "MAX":** Here we take a globalMin and globalMax value of the attribute on which HAVING is applied and update it on every row accordingly.

- **when aggregate function on HAVING attribute is "SUM" or "AVG":** Since AVG can be computed by dividing the SUM by a counter that we increment at every step by 1. This counter tells the number of rows in one group.

6. In both the above subparts, we traverse through every row of the blockified pages of the sorted Table. Simultaenously, we store four values of the RETURN attribute: min, max, sum and count (similar to how we did for HAVING attribute) 

   If the current value of the 'GroupBy' attribute is not equal to the 'previous' variable value, we check the HAVING condition, insert the simultaneously being calculated values for RETURN based on it's aggregate function and update the values for new group accordingly.

7. Lastly, since we can move out of the table while traversing, we check the same condition once again after completing all the pages and insert the values if the condition is satisfied.

## Assumptions
1. Order may or may not the preserved of the rows in output table for JOIN operations.
2. We assuming that all the entries of the table will be of integer type only. It will not work for non-integer types.
3. The resultant table of the JOIN operations will not be made permenant, it will be loaded in the table catelog and available to perform queries on it.
4. To make the queries result permenant you have to make use of EXPORT command.

## Learnings
1. We learnt how to optimize the number of block access for different types of queries.
2. We learnt how to make queries faster by using the constant extra space.

## Work Distribution

### Jeet Shah (2022201009)
- External Sorting
- Order by
- JOIN - (==, >)
- Report - External Sorting, Assumptions, Learnings

### Meet Patel (2022201002)
- External Sorting
- Order by
- JOIN - (<=, >=, <)
- Report - Order by, JOIN

### Praddyumn Shukla (2022201001)
- External Sorting Parser
- Group by
- Report - Group by
