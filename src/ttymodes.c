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

static int tty_iflag_modes[8][2] = {
  {IGNPAR, 30},
  {PARMRK, 31},
  {INPCK,  32},
  {ISTRIP, 33},
  {INLCR,  34},
  {IGNCR,  35},
  {ICRNL,  36},
  {0,      0}
};

static int tty_lflag_modes[14][2] = {
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
  {0,         0}
};

static int tty_oflag_modes[7][2] = {
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
  {0,          0}
};

static int tty_cflag_modes[15][2] = {
  {CS7,         90},
  {CS8,         91},
  {PARENB,      92},
  {PARODD,      93},
  {0,           0}
};

static int buffer_add_ttymode(ssh_buffer *tty_modes_buffer, int tty_mode[2], tcflag_t termios_flag) {
  int rc = SSH_ERROR;
  uint32_t mode_mark = tty_mode[0];
  uint8_t opcode = tty_mode[1];
  rc = buffer_add_u8(*tty_modes_buffer, opcode);
  if (rc != SSH_OK) {
    return rc;
  }

  uint32_t flag = htonl((termios_flag & mode_mark) != 0);
  rc = buffer_add_u32(*tty_modes_buffer, flag);\
  return rc;
}

static int buffer_add_tty_modes(ssh_buffer *tty_modes_buffer, int tty_modes[][2], tcflag_t termios_flag) {
  int rc = SSH_ERROR;
  int i = 0;
  do {
    rc = buffer_add_ttymode(tty_modes_buffer, tty_modes[i], termios_flag);
    i++;
    if (rc != SSH_OK) {
      return rc;
    }
  } while ( tty_modes[i][0] );
  return rc;
}

/**
 * @brief Encodes terminal modes for req-pty
 *
 * @param[in]  termios_p  The termios used to set modes for pty
 *
 * @return                A SSH buffer of Encoded terminal modes, NULL on error.
 */
ssh_buffer tty_make_modes(struct termios *termios_p) {
  int rc = SSH_ERROR;
  ssh_buffer tty_modes_buffer = ssh_buffer_new();
  if (tty_modes_buffer == NULL) {
    return NULL;
  }

#define CHECK_RET_CODE \
  if (rc != SSH_OK) {\
    ssh_buffer_free(tty_modes_buffer); \
    return NULL; \
  }

  rc = buffer_add_tty_modes(&tty_modes_buffer, tty_iflag_modes, termios_p->c_iflag);
  CHECK_RET_CODE;
  rc = buffer_add_tty_modes(&tty_modes_buffer, tty_lflag_modes, termios_p->c_lflag);
  CHECK_RET_CODE;
  rc = buffer_add_tty_modes(&tty_modes_buffer, tty_oflag_modes, termios_p->c_oflag);
  CHECK_RET_CODE;
  rc = buffer_add_tty_modes(&tty_modes_buffer, tty_cflag_modes, termios_p->c_cflag);
  CHECK_RET_CODE;

  #undef CHECK_RET_CODE

  return tty_modes_buffer;
}
