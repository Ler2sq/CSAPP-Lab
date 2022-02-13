from pwn import*
context(os="Linux",arch="x86",log_level="debug")
p=process("./bomb")
se      = lambda data               :p.send(data)
sa      = lambda delim,data         :p.sendafter(delim, data)
sl      = lambda data               :p.sendline(data)
sla     = lambda delim,data         :p.sendlineafter(delim, data)
sea     = lambda delim,data         :p.sendafter(delim, data)
rc      = lambda numb=4096          :p.recv(numb)
rl      = lambda                    :p.recvline()
ru      = lambda delims             :p.recvuntil(delims)
uu32    = lambda data               :u32(data.ljust(4, b'\x00'))
uu64    = lambda data               :u64(data.ljust(8, b'\x00'))
info    = lambda tag, addr          :p.info('======>'+tag + ': {:#x}'.format(addr))
ir      = lambda                    :p.interactive()
def dbg():
        #gdb.attach(p)
        gdb.attach(proc.pidof(p)[0])
        raw_input()
        
def exp():
	ru("which to blow yourself up. Have a nice day!\n")
	sl("Border relations with Canada have never been better.")
	
	ru("Phase 1 defused. How about the next one?\n")
	sl("1 2 4 8 16 32")
	
	ru("That's number 2.  Keep going!\n")
	sl("0 207")
	
	ru("Halfway there!\n")
	sl("0 0")
	
	ru("So you got that one.  Try this one.\n")
	#sl("0 0")
	
	a="maduiersnfotvbyl"
	b="flyers"
	c=[]
	for i in range(6):
		c.append(a.find(b[i]))
	c=list(map(lambda x:hex(x)[2:],c))
	d="".join(c)
	print(d)
	
	sl("\x09\x0f\x0e\x05\x06\x07")
	
	ru("Good work!  On to the next...\n")
	sl("4 3 2 1 6 5")
	rc()
	
if __name__=="__main__":
	exp()
	
