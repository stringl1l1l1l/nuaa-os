from greenlet import greenlet
    

data = 'abcdefgh'

def worker():
    for letter in data:
        customer_gr.switch(letter)
           
def customer():
    while 1:
        letter =  worker_gr.switch()
        print(letter)     
    
worker_gr = greenlet(worker)
customer_gr = greenlet(customer)

customer_gr.switch()