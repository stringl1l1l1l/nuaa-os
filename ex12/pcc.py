from greenlet import greenlet
    

data = 'abcdefgh'

def worker():
    for letter in data:
        calculater_gr.switch(letter)
           
def customer(arg):
    letter = arg
    while 1:
        print(letter)
        letter =  worker_gr.switch()    

def calculater(arg):
    letter = arg
    while 1:
        letter = letter.upper()
        letter = customer_gr.switch(letter)
    
worker_gr = greenlet(worker)
customer_gr = greenlet(customer)
calculater_gr = greenlet(calculater)

worker_gr.switch()