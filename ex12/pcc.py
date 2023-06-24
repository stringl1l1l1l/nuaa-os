from greenlet import greenlet
    

data = 'abcdefgh'

def worker():
    for letter in data:
        calculater_gr.switch(letter)
           
def customer():
    while 1:
        letter = calculater_gr.switch()
        print(letter)
            
def calculater():
    while 1:
        letter = worker_gr.switch()
        letter = letter.upper()
        customer_gr.switch(letter)
    
worker_gr = greenlet(worker)
customer_gr = greenlet(customer)
calculater_gr = greenlet(calculater)

customer_gr.switch()