/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2017 by Codinn
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

static int buffer_add_ttymode(ssh_buffer buffer, int tty_mode[2], tcflag_t termios_flag) {
  int rc = SSH_ERROR;
  uint32_t mode_mark = tty_mode[0];
  uint8_t opcode = tty_mode[1];
  rc = buffer_add_u8(buffer, opcode);
  if (rc != SSH_OK) {
    return rc;
  }

  uint32_t flag = htonl((termios_flag & mode_mark) != 0);
  rc = buffer_add_u32(buffer, flag);\
  return rc;
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
    return termios.c_oflag;
  }
  return 0;
}

static int buffer_add_tty_modes(ssh_buffer buffer, int tty_modes[][2], struct termios termios) {
  int rc = SSH_ERROR;
  int i = 0;
  do {
    tcflag_t termios_flag = get_termios_flag(termios, tty_modes[i][1]);
    rc = buffer_add_ttymode(buffer, tty_modes[i], termios_flag);
    i++;
    if (rc != SSH_OK) {
      return rc;
    }
  } while ( tty_modes[i][0] );
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

  rc = buffer_add_tty_modes(buffer, tty_mode_flags, *termios_p);

  if (rc != SSH_OK) {
    ssh_buffer_free(buffer);
    return NULL;
  }

  return buffer;
}
