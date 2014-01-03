-LifeTracker-
Jing Lin and Matthew Laikhram
==============================

LifeTracker is an easy way to store important information and keep it organized. Features in LifeTracker include: 

1. Adding, Deleting, and Editing records in LifeTracker.
2. Adding and Deleting Categories
3. Searching for records in the SearchBox. 
4. Scroll through the records of every category with the up/down arrow keys and alternate between the categories and 
records section with the left/right arrow keys.
5. Sort the records chronologically or alphabetically.

To run LifeTracker, in the terminal, type: 

    gcc mystore.c -o mystore
    gcc myuiscreen.c -o myuiscreen
    ./myuiscreen

Note that mystore has been updated from Mr. Brooks' previous version. The only function that 
is different is the add function which now takes three arguments in the form: 

    ./mystore add SUBJECT BODY CATEGORY

Important functions to know include: <br>
    F2 - To Add a record, hit F2. The user is then taken to another addScreen where the parameters for a new record can be inputted.
    A category is created through this way. <br>
    F3 - To Delete a record, place the cursor over that record and hit F3 which takes the user to another delete screen
    to ascertain that the user really wants to delete the record. From there, the user can press F3 again, which deletes the record.
    If the category to which that record belonged to now has no records, then that cateogry is deleted. <br>
    F4 - To Edit a record, place the cursor over that record and hit F4. The user is then taken to another screen where he/she can Edit
    the contents of the CATEGORY, SUBJECT, and BODY. <br>
    F6 - Toggle switch between sorting the records alphabetically or chronologically. <br>
    F7 - Takes the user to the SearchBox where the user can type in the keywords for searching records that match specific parameters. F7 is also a toggle switch between the SearchBox and the the first record in the category the user was in before he/she entered the SearchBox. <br>
    F9 - Universal Exit key. If the user wants to discard the Edits made to a record, hit F9. If the user wants to not add a new record he/she has created with F2, hit F9. If the user wants to exit the delete screen, hit F9. If the user wants to exit the SearchBox, hit F9. If the user wants to exit LifeTracker, hit F9. <br>