/*
 * this file is only intended for including from channels.c.
 */

#define TTYMODE(NAME, FIELD, OP) \
              rc = buffer_add_u8(ttymodes_buffer, OP);\
              if (rc != SSH_OK) {\
                ssh_set_error_oom(session);\
                goto error;\
              }\
              ttymode_flag = htonl((tios.FIELD & NAME) != 0);\
              rc = buffer_add_u32(ttymodes_buffer, ttymode_flag);\
              if (rc != SSH_OK) {\
                ssh_set_error_oom(session);\
                goto error;\
              }

/* name, field, op */
TTYMODE(IGNPAR,	c_iflag, 30)
TTYMODE(PARMRK,	c_iflag, 31)
TTYMODE(INPCK,	c_iflag, 32)
TTYMODE(ISTRIP,	c_iflag, 33)
TTYMODE(INLCR,	c_iflag, 34)
TTYMODE(IGNCR,	c_iflag, 35)
TTYMODE(ICRNL,	c_iflag, 36)

#if defined(IUCLC)
TTYMODE(IUCLC,	c_iflag, 37)
#endif

TTYMODE(IXON,	c_iflag, 38)
TTYMODE(IXANY,	c_iflag, 39)
TTYMODE(IXOFF,	c_iflag, 40)

#ifdef IMAXBEL
TTYMODE(IMAXBEL,c_iflag, 41)
#endif /* IMAXBEL */
#ifdef IUTF8
TTYMODE(IUTF8,  c_iflag, 42)
#endif /* IUTF8 */

TTYMODE(ISIG,	c_lflag, 50)
TTYMODE(ICANON,	c_lflag, 51)

#ifdef XCASE
TTYMODE(XCASE,	c_lflag, 52)
#endif

TTYMODE(ECHO,	c_lflag, 53)
TTYMODE(ECHOE,	c_lflag, 54)
TTYMODE(ECHOK,	c_lflag, 55)
TTYMODE(ECHONL,	c_lflag, 56)
TTYMODE(NOFLSH,	c_lflag, 57)
TTYMODE(TOSTOP,	c_lflag, 58)

#ifdef IEXTEN
TTYMODE(IEXTEN, c_lflag, 59)
#endif /* IEXTEN */
#if defined(ECHOCTL)
TTYMODE(ECHOCTL,c_lflag, 60)
#endif /* ECHOCTL */
#ifdef ECHOKE
TTYMODE(ECHOKE,	c_lflag, 61)
#endif /* ECHOKE */
#if defined(PENDIN)
TTYMODE(PENDIN,	c_lflag, 62)
#endif /* PENDIN */

TTYMODE(OPOST,	c_oflag, 70)

#if defined(OLCUC)
TTYMODE(OLCUC,	c_oflag, 71)
#endif
#ifdef ONLCR
TTYMODE(ONLCR,	c_oflag, 72)
#endif
#ifdef OCRNL
TTYMODE(OCRNL,	c_oflag, 73)
#endif
#ifdef ONOCR
TTYMODE(ONOCR,	c_oflag, 74)
#endif
#ifdef ONLRET
TTYMODE(ONLRET,	c_oflag, 75)
#endif

TTYMODE(CS7,	c_cflag, 90)
TTYMODE(CS8,	c_cflag, 91)
TTYMODE(PARENB,	c_cflag, 92)
TTYMODE(PARODD,	c_cflag, 93)

#undef TTYMODE
