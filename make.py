import string
import random
def make():
    print "drop table test;"
    print "create table test (a int, b int unique, c float, d float unique, e char (100), primary key(e));"
    for i in xrange(100):
        print "insert into test values (%d, %d, %f, %f, '%s');" % (random.randint(0, 100), random.randint(0, 100), float(random.randint(1, 1000)) / random.randint(1, 10), float(random.randint(1, 1000)) / random.randint(1, 10), ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6)))

if __name__ == "__main__":
    make()