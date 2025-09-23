;
;   this program reads in an array of positive numbers 
;   and computes the sum of the elements. to end array input user types a -1 
;
;   IDX = index register
;   ACC = accumulator
;
220040      ; 0 - load contents of location 40 in IDX (=45) 
100041      ; 1 - input a value in location 41
200041      ; 2 - load value from 41 in ACC
410007      ; 3 - if value < 0 then we are done inputting array, continue with adding
260000      ; 4 - store contents at memory location specified in IDX
380000      ; 5 - increase IDX by 1
400001      ; 6 - get next value ( loop )
430000      ; 7 - swap ACC and IDX
320040      ; 8 - subtract ACC from contents of location 40 ( this result is the number of lements read in!!!)
420025      ; 9 - if this result is 0 no numbers were entered, go to 25 ( special case)
300040      ; 10 - add back what you subtracted
430000      ; 11 - and swap IDX and ACC 
210000      ; 12 - set ACC to 0
390000      ; 13 - reduce IDX ( IDX is currently referring to last element of array)
310000      ; 14 - add the contents of memroy location referred to by IDX to ACC
430000      ; 15 - in the next few statements we exchange IDX and ACC, subtract
320040      ; 16 - IDX from the base address of array (45) and if its 0
420021      ; 17 - then we've processed all elements of array and can now print sum
300040      ; 18 - next to restore IDX to original value and swap IDX and ACC
430000      ; 19 - remember ACC is currently holding the sum
400013      ; 20 - branch to location 13 ( process next element in array)
430000      ; 21 - remember IDX is currently holding the SUM
250041      ; 22 - now ACC holds sum and we can print it
110041      ; 23
450001      ; 24 - halt and dump core
-210001     ; 25 - this code handles special case of no input displays -1
250041      ; 26 - puts -1 in ACC and prints it out
110041      ; 27
450001      ; 28 - halt program and dump core
000000
000000
000000
000000
000000
000000
000000
000000
000000
000000
000000
000045      ; 40 - memory location 45 is where first element of array will go
go


