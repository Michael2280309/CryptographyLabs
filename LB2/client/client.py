import socket
import time
import random

HOST = "127.0.0.1"
PORT = 8080
BUFFER_SIZE = 512

def die():
    return random.randint(7, 32768 - 1)

def rsa_pow_mod(a, b, c):
    power = b
    d = a
    res = 1
    while power:
        if power & 1:
            res = (res * d) % c
        d = (d * d) % c
        power = power >> 1
    return res % c

def rsa_gcd(u, v):
    if u == 0:
        return v
    if v == 0:
        return u

    shift = 0
    while (u | v) & 1 == 0:
        u >>= 1
        v >>= 1
        shift += 1

    while u & 1 == 0:
        u >>= 1

    while v != 0:
        while v & 1 == 0:
            v >>= 1
        if u > v:
            u, v = v, u
        v = v - u

    return u << shift

def rsa_gen_E(fi):
    for e in range(2, fi):
        if rsa_gcd(e, fi) == 1:
            return e
    return -1

def rsa_D_calculate(E, fi):
    d = 1
    k = 1
    while True:
        k = k + fi
        if k % E == 0:
            d = k // E
            return d

def rsa_key_gen():
    p = rsa_prime()
    q = rsa_prime()
    while p == q:
        p = rsa_prime()
        q = rsa_prime()
    n = p * q
    fi = (p - 1) * (q - 1)
    E = rsa_gen_E(fi)
    D = rsa_D_calculate(E, fi)
    return E, D, n

def rsa_encrypt(a, E, n):
    return rsa_pow_mod(a, E, n)

def rsa_decrypt(b, D, n):
    return rsa_pow_mod(b, D, n)

def rsa_prime():
    nr = die()

    few_primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251]

    while True:
        div = any(nr % prime == 0 for prime in few_primes)
        if not div and rsa_miller_rabin_test(nr, 16):
            return nr
        nr = die()

def rsa_miller_rabin_test(n, k):
    t = n - 1
    while t % 2 == 0:
        t //= 2

    while k > 0:
        a = random.randint(2, n - 1)
        x = rsa_pow_mod(a, t, n)
        if x == 1 or x == n - 1:
            k -= 1
            continue
        t1 = t
        while True:
            if t1 == n - 1:
                break
            if x == 1:
                break
            if x == n - 1:
                break

            x = rsa_pow_mod(x, 2, n)
            t1 *= 2
        if x != n - 1 and t1 % 2 == 0:
            return 0
        k -= 1

    return 1

if __name__ == "__main__":
    random.seed(time.time())
    # создаем сокет для клиента
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # подключаемся к серверу
    client_socket.connect((HOST, PORT))

    e, d, n = rsa_key_gen()
    buffer = f"{e} {n}"
    
    client_socket.sendall(buffer.encode())
    
    buffer = ''

    buffer = client_socket.recv(BUFFER_SIZE)
    test_block_encr = int(buffer.replace(b'\x00', b''))
    test_block = rsa_decrypt(test_block_encr, d, n)
    buffer = f"{test_block}"
    client_socket.sendall(buffer.encode())

    buffer = ""

    buffer = client_socket.recv(BUFFER_SIZE)
    skey = buffer.split()
    se = int(skey[0].replace(b'\x00', b''))
    sn = int(skey[1].replace(b'\x00', b''))
    test_en = die()
    test_en_out = rsa_encrypt(test_en, se, sn)
    buffer = f"{test_en_out}"
    client_socket.sendall(buffer.encode())
    buffer = ""

    buffer = client_socket.recv(BUFFER_SIZE)
    got_test = int(buffer.replace(b'\x00', b''))
    if got_test == test_en:
         succ = "Successful server authentication!"
         print(f"Client says: {succ}")
    else:
        fail = "Server authentication failed!"
        print(f"Client says: {fail}")
        client_socket.close()

    client_socket.close()
