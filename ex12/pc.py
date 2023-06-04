from greenlet import greenlet
    

data = 'abcdefgh'

def worker():
    for letter in data:
        customer_gr.switch(letter)
           
def customer(arg):
    letter = arg
    while 1:
        print(letter)
        letter =  worker_gr.switch()      
    
worker_gr = greenlet(worker)
customer_gr = greenlet(customer)

worker_gr.switch()