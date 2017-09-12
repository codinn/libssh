/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2017 by Hu Xiaomao <hu@codinn.com>
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */
#include <termios.h>

#include <libssh/priv.h>
#include <libssh/libssh.h>
#include <libssh/buffer.h>

#define TTY_OP_ISPEED	128
#define TTY_OP_OSPEED	129

static int tty_mode_cc[20][2] = {
  /* control chars */
  {VINTR,    1},
  {VQUIT,    2},
  {VERASE,   3},
#if defined(VKILL)
  {VKILL,    4},
#endif /* VKILL */
  {VEOF,     5},
#if defined(VEOL)
  {VEOL,     6},
#endif /* VEOL */
#ifdef VEOL2
  {VEOL2,    7},
#endif /* VEOL2 */
  {VSTART,   8},
  {VSTOP,    9},
#if defined(VSUSP)
  {VSUSP,    10},
#endif /* VSUSP */
#if defined(VDSUSP)
  {VDSUSP,   11},
#endif /* VDSUSP */
#if defined(VREPRINT)
  {VREPRINT, 12},
#endif /* VREPRINT */
#if defined(VWERASE)
  {VWERASE,  13},
#endif /* VWERASE */
#if defined(VLNEXT)
  {VLNEXT,   14},
#endif /* VLNEXT */
#if defined(VFLUSH)
  {VFLUSH,   15},
#endif /* VFLUSH */
#ifdef VSWTCH
  {VSWTCH,   16},
#endif /* VSWTCH */
#if defined(VSTATUS)
  {VSTATUS,  17},
#endif /* VSTATUS */
#ifdef VDISCARD
  {VDISCARD, 18},
#endif /* VDISCARD */

  /* end flag */
  {0,      0}
};

static int tty_mode_flags[36][2] = {
  /* c_iflag */
  {IGNPAR, 30},
  {PARMRK, 31},
  {INPCK,  32},
  {ISTRIP, 33},
  {INLCR,  34},
  {IGNCR,  35},
  {ICRNL,  36},
#if defined(IUCLC)
  {IUCLC,  37},
#endif
  {IXON,   38},
  {IXANY,  39},
  {IXOFF,  40},
#ifdef IMAXBEL
  {IMAXBEL,41},
#endif /* IMAXBEL */
#ifdef IUTF8
  {IUTF8,  42},
#endif /* IUTF8 */

  /* c_lflag */
  {ISIG,      50},
  {ICANON,    51},
#ifdef XCASE
  {XCASE,     52},
#endif
  {ECHO,      53},
  {ECHOE,     54},
  {ECHOK,     55},
  {ECHONL,    56},
  {NOFLSH,    57},
  {TOSTOP,    58},
#ifdef IEXTEN
  {IEXTEN,    59},
#endif /* IEXTEN */
#if defined(ECHOCTL)
  {ECHOCTL,   60},
#endif /* ECHOCTL */
#ifdef ECHOKE
  {ECHOKE,    61},
#endif /* ECHOKE */
#if defined(PENDIN)
  {PENDIN,    62},
#endif /* PENDIN */

  /* c_oflag */
  {OPOST,      70},
#if defined(OLCUC)
  {OLCUC,      71},
#endif
#ifdef ONLCR
  {ONLCR,      72},
#endif
#ifdef OCRNL
  {OCRNL,      73},
#endif
#ifdef ONOCR
  {ONOCR,      74},
#endif
#ifdef ONLRET
  {ONLRET,     75},
#endif

  /* c_cflag */
  {CS7,         90},
  {CS8,         91},
  {PARENB,      92},
  {PARODD,      93},

  /* end flag */
  {0,      0}
};

static int speed_to_baud(speed_t speed) {
  switch (speed) {
    case B0:
      return 0;
    case B50:
      return 50;
    case B75:
      return 75;
    case B110:
      return 110;
    case B134:
      return 134;
    case B150:
      return 150;
    case B200:
      return 200;
    case B300:
      return 300;
    case B600:
      return 600;
    case B1200:
      return 1200;
    case B1800:
      return 1800;
    case B2400:
      return 2400;
    case B4800:
      return 4800;
    case B9600:
      return 9600;

#ifdef B19200
    case B19200:
      return 19200;
#else /* B19200 */
#ifdef EXTA
    case EXTA:
      return 19200;
#endif /* EXTA */
#endif /* B19200 */

#ifdef B38400
    case B38400:
      return 38400;
#else /* B38400 */
#ifdef EXTB
    case EXTB:
      return 38400;
#endif /* EXTB */
#endif /* B38400 */

#ifdef B7200
    case B7200:
      return 7200;
#endif /* B7200 */
#ifdef B14400
    case B14400:
      return 14400;
#endif /* B14400 */
#ifdef B28800
    case B28800:
      return 28800;
#endif /* B28800 */
#ifdef B57600
    case B57600:
      return 57600;
#endif /* B57600 */
#ifdef B76800
    case B76800:
      return 76800;
#endif /* B76800 */
#ifdef B115200
    case B115200:
      return 115200;
#endif /* B115200 */
#ifdef B230400
    case B230400:
      return 230400;
#endif /* B230400 */
    default:
      return 9600;
  }
}

static u_int special_char_encode(cc_t c) {
#ifdef _POSIX_VDISABLE
  if (c == _POSIX_VDISABLE)
  return 255;
#endif
  return c;
}

static int buffer_add_ttymode(ssh_buffer buffer, uint8_t opcode, u_int value) {
  int rc = SSH_ERROR;
    
  rc = buffer_add_u8(buffer, opcode);
  if (rc != SSH_OK) {
    return rc;
  }
    
  uint32_t flag = htonl(value);
  rc = buffer_add_u32(buffer, flag);
  return rc;
}

static int buffer_add_ttymode_speed(ssh_buffer buffer, uint8_t opcode, speed_t speed) {
  return buffer_add_ttymode(buffer, opcode, speed_to_baud(speed));
}

static int buffer_add_ttymode_cc(ssh_buffer buffer, int tty_cc[2], cc_t termios_cc[NCCS]) {
  uint8_t name = tty_cc[0];
  uint8_t opcode = tty_cc[1];
    
  return buffer_add_ttymode(buffer, opcode, special_char_encode(termios_cc[name]));
}

static int buffer_add_ttymode_flag(ssh_buffer buffer, int tty_mode[2], tcflag_t termios_flag) {
  uint32_t mode_mark = tty_mode[0];
  uint8_t opcode = tty_mode[1];
    
  return buffer_add_ttymode(buffer, opcode, (termios_flag & mode_mark) != 0);
}

static inline tcflag_t get_termios_flag(struct termios termios, uint8_t opcode) {
  if (opcode >= 30 && opcode < 50) {
    return termios.c_iflag;
  }
  if (opcode >= 50 && opcode < 70) {
    return termios.c_lflag;
  }
  if (opcode >= 70 && opcode < 90) {
    return termios.c_oflag;
  }
  if (opcode >= 90 && opcode < 100) {
    return termios.c_cflag;
  }
  return 0;
}

static int buffer_add_tty_modes(ssh_buffer buffer, struct termios termios) {
  int rc = SSH_ERROR;
  int i = 0;
    
  buffer_add_ttymode_speed(buffer, TTY_OP_ISPEED, termios.c_ispeed);
  buffer_add_ttymode_speed(buffer, TTY_OP_OSPEED, termios.c_ospeed);

  do {
    tcflag_t termios_flag = get_termios_flag(termios, tty_mode_flags[i][1]);
    rc = buffer_add_ttymode_flag(buffer, tty_mode_flags[i], termios_flag);
    i++;
    if (rc != SSH_OK) {
      return rc;
    }
  } while (tty_mode_flags[i][0]);

  i = 0;
  do {
    rc = buffer_add_ttymode_cc(buffer, tty_mode_cc[i], termios.c_cc);
    i++;
    if (rc != SSH_OK) {
      return rc;
    }
  } while (tty_mode_cc[i][1]);
  return rc;
}

/**
 * @brief Encodes terminal modes
 *
 * @param[in]  termios_p  The termios used to set modes for tty
 *
 * @return                A SSH buffer of Encoded terminal modes, NULL on error.
 */
ssh_buffer tty_make_modes(struct termios *termios_p) {
  int rc = SSH_ERROR;

  ssh_buffer buffer = ssh_buffer_new();
  if (buffer == NULL) {
    return NULL;
  }

  rc = buffer_add_tty_modes(buffer, *termios_p);

  if (rc != SSH_OK) {
    ssh_buffer_free(buffer);
    return NULL;
  }

  return buffer;
}
