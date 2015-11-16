
[31;1merror: file not found or empty: osdefs.k[m
[32m[?7lboot.l:24   [0m (abort)
[?7h[32m[?7lboot.l:993  [0m (error "file not found or empty: " name)
[?7h[32m[?7lboot.l:993  [0m (Fixed<or> exps (error "file not found or empty: " name))
[?7h[32m[?7lboot.l:992  [0m (Fixed<let> ((exps (find-and-read name))) (Fixed<or> exps (error "file not found or empty: " name)) (Fixed<let> ((_list_ exps)) (Fixed<while> _list_ (Fixed<let> ((expr (car _list_))) (pval expr)) (Fixed<set> _list_ (cdr _list_)))))
[?7h[32m[?7lboot.l:1002 [0m (load name)
[?7h[32m[?7lboot.l:1000 [0m (Fixed<let> () (Fixed<set> *loaded* (cons name *loaded*)) (load name))
[?7h[32m[?7lboot.l:999  [0m (Fixed<or> (member? name *loaded*) (Fixed<let> () (Fixed<set> *loaded* (cons name *loaded*)) (load name)))
[?7h[32m[?7lemit.l:1    [0m (require "osdefs.k")
[?7h