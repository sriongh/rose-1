void
ngx_process_get_status(void)
   {


#if 0
  // Original failing code:
     int status;
     if ((((__extension__ (((union ABC { __typeof(status) __in; int __i; }) { .__in = (status) }).__i))) & 0x7f))
        {
        }
#endif

#if 0
     int status;
     int x = ((((__extension__ (((union { __typeof(status) __in; int __i; }) { .__in = (status) }).__i))) & 0x7f));
#endif

     int status;
     if ( (((union { __typeof(status) __in; int __i; }) { .__in = 42 }).__i) == 0)
        {
        }


   }
