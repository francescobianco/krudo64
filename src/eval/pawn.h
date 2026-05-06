#pragma once
/* ────────────────────────────────────────────────────────────────────────────
 * src/eval/pawn.h  —  Struttura dei pedoni per colonna
 *
 * Indice: maschera a 8 bit delle COLONNE occupate da almeno un pedone.
 *   bit 0 = col. a  …  bit 7 = col. h
 *   1 = il lato ha almeno un pedone su quella colonna (la traversa non conta)
 *
 * Punteggio dal punto di vista del proprietario (positivo = meglio).
 *
 * Criteri tematici:
 *   - Isole in più    : -15 cp per isola extra
 *     (più isole = più colonne semiaperte per il nemico, meno supporto reciproco)
 *   - Pedone isolato  : -20 cp
 *     (nessun pedone adiacente: bersaglio fisso, non può essere difeso da pedoni)
 *   - Colonna d / e   : +5 cp  (centro diretto → controllo spazio massimo)
 *   - Colonna c / f   : +2 cp  (semi-centro → sviluppo più flessibile)
 *
 * Uso in eval.c:
 *   uint8_t wm = pawn_file_mask(b->pieces[WHITE][PAWN]);
 *   uint8_t bm = pawn_file_mask(b->pieces[BLACK][PAWN]);
 *   int pawn_struct = PAWN_STRUCTURE[wm] - PAWN_STRUCTURE[bm];
 *
 * Come leggere il pattern nei commenti:
 *   'p' = colonna occupata,  '.' = colonna libera  (da a a h, sinistra→destra)
 * ──────────────────────────────────────────────────────────────────────────── */

/* ── Tutti i 256 pattern ──────────────────────────────────────────────────── */

static const int PAWN_STRUCTURE[256] = {

    /* 0x00  ........ */     0,  /* nessun pedone */
    /* 0x01  p....... */   -20,
    /* 0x02  .p...... */   -20,
    /* 0x03  pp...... */     0,
    /* 0x04  ..p..... */   -18,
    /* 0x05  p.p..... */   -53,
    /* 0x06  .pp..... */     2,
    /* 0x07  ppp..... */     2,

    /* 0x08  ...p.... */   -15,
    /* 0x09  p..p.... */   -50,
    /* 0x0A  .p.p.... */   -50,
    /* 0x0B  pp.p.... */   -30,
    /* 0x0C  ..pp.... */     7,
    /* 0x0D  p.pp.... */   -28,
    /* 0x0E  .ppp.... */     7,
    /* 0x0F  pppp.... */     7,

    /* 0x10  ....p... */   -15,
    /* 0x11  p...p... */   -50,
    /* 0x12  .p..p... */   -50,
    /* 0x13  pp..p... */   -30,
    /* 0x14  ..p.p... */   -48,
    /* 0x15  p.p.p... */   -83,
    /* 0x16  .pp.p... */   -28,
    /* 0x17  ppp.p... */   -28,

    /* 0x18  ...pp... */    10,  /* soli d+e (centro ideale) */
    /* 0x19  p..pp... */   -25,
    /* 0x1A  .p.pp... */   -25,
    /* 0x1B  pp.pp... */    -5,
    /* 0x1C  ..ppp... */    12,
    /* 0x1D  p.ppp... */   -23,
    /* 0x1E  .pppp... */    12,
    /* 0x1F  ppppp... */    12,

    /* 0x20  .....p.. */   -18,
    /* 0x21  p....p.. */   -53,
    /* 0x22  .p...p.. */   -53,
    /* 0x23  pp...p.. */   -33,
    /* 0x24  ..p..p.. */   -51,  /* solo c+f */
    /* 0x25  p.p..p.. */   -86,
    /* 0x26  .pp..p.. */   -31,
    /* 0x27  ppp..p.. */   -31,

    /* 0x28  ...p.p.. */   -48,
    /* 0x29  p..p.p.. */   -83,
    /* 0x2A  .p.p.p.. */   -83,
    /* 0x2B  pp.p.p.. */   -63,
    /* 0x2C  ..pp.p.. */   -26,
    /* 0x2D  p.pp.p.. */   -61,
    /* 0x2E  .ppp.p.. */   -26,
    /* 0x2F  pppp.p.. */   -26,

    /* 0x30  ....pp.. */     7,
    /* 0x31  p...pp.. */   -28,
    /* 0x32  .p..pp.. */   -28,
    /* 0x33  pp..pp.. */    -8,
    /* 0x34  ..p.pp.. */   -26,
    /* 0x35  p.p.pp.. */   -61,
    /* 0x36  .pp.pp.. */    -6,
    /* 0x37  ppp.pp.. */    -6,

    /* 0x38  ...ppp.. */    12,
    /* 0x39  p..ppp.. */   -23,
    /* 0x3A  .p.ppp.. */   -23,
    /* 0x3B  pp.ppp.. */    -3,
    /* 0x3C  ..pppp.. */    14,  /* c+d+e+f (centro massimo) */
    /* 0x3D  p.pppp.. */   -21,
    /* 0x3E  .ppppp.. */    14,
    /* 0x3F  pppppp.. */    14,

    /* 0x40  ......p. */   -20,
    /* 0x41  p.....p. */   -55,
    /* 0x42  .p....p. */   -55,
    /* 0x43  pp....p. */   -35,
    /* 0x44  ..p...p. */   -53,
    /* 0x45  p.p...p. */   -88,
    /* 0x46  .pp...p. */   -33,
    /* 0x47  ppp...p. */   -33,

    /* 0x48  ...p..p. */   -50,
    /* 0x49  p..p..p. */   -85,
    /* 0x4A  .p.p..p. */   -85,
    /* 0x4B  pp.p..p. */   -65,
    /* 0x4C  ..pp..p. */   -28,
    /* 0x4D  p.pp..p. */   -63,
    /* 0x4E  .ppp..p. */   -28,
    /* 0x4F  pppp..p. */   -28,

    /* 0x50  ....p.p. */   -50,
    /* 0x51  p...p.p. */   -85,
    /* 0x52  .p..p.p. */   -85,
    /* 0x53  pp..p.p. */   -65,
    /* 0x54  ..p.p.p. */   -83,
    /* 0x55  p.p.p.p. */  -118,  /* a+c+e+g (tutto isolato) */
    /* 0x56  .pp.p.p. */   -63,
    /* 0x57  ppp.p.p. */   -63,

    /* 0x58  ...pp.p. */   -25,
    /* 0x59  p..pp.p. */   -60,
    /* 0x5A  .p.pp.p. */   -60,
    /* 0x5B  pp.pp.p. */   -40,
    /* 0x5C  ..ppp.p. */   -23,
    /* 0x5D  p.ppp.p. */   -58,
    /* 0x5E  .pppp.p. */   -23,
    /* 0x5F  ppppp.p. */   -23,

    /* 0x60  .....pp. */     2,
    /* 0x61  p....pp. */   -33,
    /* 0x62  .p...pp. */   -33,
    /* 0x63  pp...pp. */   -13,
    /* 0x64  ..p..pp. */   -31,
    /* 0x65  p.p..pp. */   -66,
    /* 0x66  .pp..pp. */   -11,
    /* 0x67  ppp..pp. */   -11,

    /* 0x68  ...p.pp. */   -28,
    /* 0x69  p..p.pp. */   -63,
    /* 0x6A  .p.p.pp. */   -63,
    /* 0x6B  pp.p.pp. */   -43,
    /* 0x6C  ..pp.pp. */    -6,
    /* 0x6D  p.pp.pp. */   -41,
    /* 0x6E  .ppp.pp. */    -6,
    /* 0x6F  pppp.pp. */    -6,

    /* 0x70  ....ppp. */     7,
    /* 0x71  p...ppp. */   -28,
    /* 0x72  .p..ppp. */   -28,
    /* 0x73  pp..ppp. */    -8,
    /* 0x74  ..p.ppp. */   -26,
    /* 0x75  p.p.ppp. */   -61,
    /* 0x76  .pp.ppp. */    -6,
    /* 0x77  ppp.ppp. */    -6,

    /* 0x78  ...pppp. */    12,
    /* 0x79  p..pppp. */   -23,
    /* 0x7A  .p.pppp. */   -23,
    /* 0x7B  pp.pppp. */    -3,
    /* 0x7C  ..ppppp. */    14,
    /* 0x7D  p.ppppp. */   -21,
    /* 0x7E  .pppppp. */    14,  /* b..g senza a e h (struttura solida) */
    /* 0x7F  ppppppp. */    14,

    /* 0x80  .......p */   -20,
    /* 0x81  p......p */   -55,  /* solo a+h (pedoni fuori gioco) */
    /* 0x82  .p.....p */   -55,
    /* 0x83  pp.....p */   -35,
    /* 0x84  ..p....p */   -53,
    /* 0x85  p.p....p */   -88,
    /* 0x86  .pp....p */   -33,
    /* 0x87  ppp....p */   -33,

    /* 0x88  ...p...p */   -50,
    /* 0x89  p..p...p */   -85,
    /* 0x8A  .p.p...p */   -85,
    /* 0x8B  pp.p...p */   -65,
    /* 0x8C  ..pp...p */   -28,
    /* 0x8D  p.pp...p */   -63,
    /* 0x8E  .ppp...p */   -28,
    /* 0x8F  pppp...p */   -28,

    /* 0x90  ....p..p */   -50,
    /* 0x91  p...p..p */   -85,
    /* 0x92  .p..p..p */   -85,
    /* 0x93  pp..p..p */   -65,
    /* 0x94  ..p.p..p */   -83,
    /* 0x95  p.p.p..p */  -118,
    /* 0x96  .pp.p..p */   -63,
    /* 0x97  ppp.p..p */   -63,

    /* 0x98  ...pp..p */   -25,
    /* 0x99  p..pp..p */   -60,
    /* 0x9A  .p.pp..p */   -60,
    /* 0x9B  pp.pp..p */   -40,
    /* 0x9C  ..ppp..p */   -23,
    /* 0x9D  p.ppp..p */   -58,
    /* 0x9E  .pppp..p */   -23,
    /* 0x9F  ppppp..p */   -23,

    /* 0xA0  .....p.p */   -53,
    /* 0xA1  p....p.p */   -88,
    /* 0xA2  .p...p.p */   -88,
    /* 0xA3  pp...p.p */   -68,
    /* 0xA4  ..p..p.p */   -86,
    /* 0xA5  p.p..p.p */  -121,
    /* 0xA6  .pp..p.p */   -66,
    /* 0xA7  ppp..p.p */   -66,

    /* 0xA8  ...p.p.p */   -83,
    /* 0xA9  p..p.p.p */  -118,
    /* 0xAA  .p.p.p.p */  -118,  /* b+d+f+h (tutto isolato) */
    /* 0xAB  pp.p.p.p */   -98,
    /* 0xAC  ..pp.p.p */   -61,
    /* 0xAD  p.pp.p.p */   -96,
    /* 0xAE  .ppp.p.p */   -61,
    /* 0xAF  pppp.p.p */   -61,

    /* 0xB0  ....pp.p */   -28,
    /* 0xB1  p...pp.p */   -63,
    /* 0xB2  .p..pp.p */   -63,
    /* 0xB3  pp..pp.p */   -43,
    /* 0xB4  ..p.pp.p */   -61,
    /* 0xB5  p.p.pp.p */   -96,
    /* 0xB6  .pp.pp.p */   -41,
    /* 0xB7  ppp.pp.p */   -41,

    /* 0xB8  ...ppp.p */   -23,
    /* 0xB9  p..ppp.p */   -58,
    /* 0xBA  .p.ppp.p */   -58,
    /* 0xBB  pp.ppp.p */   -38,
    /* 0xBC  ..pppp.p */   -21,
    /* 0xBD  p.pppp.p */   -56,
    /* 0xBE  .ppppp.p */   -21,
    /* 0xBF  pppppp.p */   -21,

    /* 0xC0  ......pp */     0,
    /* 0xC1  p.....pp */   -35,
    /* 0xC2  .p....pp */   -35,
    /* 0xC3  pp....pp */   -15,
    /* 0xC4  ..p...pp */   -33,
    /* 0xC5  p.p...pp */   -68,
    /* 0xC6  .pp...pp */   -13,
    /* 0xC7  ppp...pp */   -13,

    /* 0xC8  ...p..pp */   -30,
    /* 0xC9  p..p..pp */   -65,
    /* 0xCA  .p.p..pp */   -65,
    /* 0xCB  pp.p..pp */   -45,
    /* 0xCC  ..pp..pp */    -8,
    /* 0xCD  p.pp..pp */   -43,
    /* 0xCE  .ppp..pp */    -8,
    /* 0xCF  pppp..pp */    -8,

    /* 0xD0  ....p.pp */   -30,
    /* 0xD1  p...p.pp */   -65,
    /* 0xD2  .p..p.pp */   -65,
    /* 0xD3  pp..p.pp */   -45,
    /* 0xD4  ..p.p.pp */   -63,
    /* 0xD5  p.p.p.pp */   -98,
    /* 0xD6  .pp.p.pp */   -43,
    /* 0xD7  ppp.p.pp */   -43,

    /* 0xD8  ...pp.pp */    -5,
    /* 0xD9  p..pp.pp */   -40,
    /* 0xDA  .p.pp.pp */   -40,
    /* 0xDB  pp.pp.pp */   -20,
    /* 0xDC  ..ppp.pp */    -3,
    /* 0xDD  p.ppp.pp */   -38,
    /* 0xDE  .pppp.pp */    -3,
    /* 0xDF  ppppp.pp */    -3,

    /* 0xE0  .....ppp */     2,
    /* 0xE1  p....ppp */   -33,
    /* 0xE2  .p...ppp */   -33,
    /* 0xE3  pp...ppp */   -13,
    /* 0xE4  ..p..ppp */   -31,
    /* 0xE5  p.p..ppp */   -66,
    /* 0xE6  .pp..ppp */   -11,
    /* 0xE7  ppp..ppp */   -11,

    /* 0xE8  ...p.ppp */   -28,
    /* 0xE9  p..p.ppp */   -63,
    /* 0xEA  .p.p.ppp */   -63,
    /* 0xEB  pp.p.ppp */   -43,
    /* 0xEC  ..pp.ppp */    -6,
    /* 0xED  p.pp.ppp */   -41,
    /* 0xEE  .ppp.ppp */    -6,
    /* 0xEF  pppp.ppp */    -6,

    /* 0xF0  ....pppp */     7,
    /* 0xF1  p...pppp */   -28,
    /* 0xF2  .p..pppp */   -28,
    /* 0xF3  pp..pppp */    -8,
    /* 0xF4  ..p.pppp */   -26,
    /* 0xF5  p.p.pppp */   -61,
    /* 0xF6  .pp.pppp */    -6,
    /* 0xF7  ppp.pppp */    -6,

    /* 0xF8  ...ppppp */    12,
    /* 0xF9  p..ppppp */   -23,
    /* 0xFA  .p.ppppp */   -23,
    /* 0xFB  pp.ppppp */    -3,
    /* 0xFC  ..pppppp */    14,
    /* 0xFD  p.pppppp */   -21,
    /* 0xFE  .ppppppp */    14,
    /* 0xFF  pppppppp */    14   /* tutte le colonne coperte */

};

/* ── pawn_file_mask ───────────────────────────────────────────────────────── *
 * Riduce una Bitboard di pedoni a una maschera a 8 bit dove                  *
 * il bit f vale 1 sse c'è almeno un pedone sulla colonna f.                  *
 * Algoritmo: fold gerarchico OR dei byte del bitboard.                       *
 * Nessuna iterazione, nessuna lookup aggiuntiva — solo 3 shift e 3 OR.       */
static inline uint8_t pawn_file_mask(Bitboard pawns)
{
    uint64_t f = (uint64_t)pawns;
    f |= f >> 32;   /* OR righe 4-7 in 0-3 */
    f |= f >> 16;   /* OR righe 2-3 in 0-1 */
    f |= f >>  8;   /* OR riga  1   in 0   */
    return (uint8_t)(f & 0xFF);
}
