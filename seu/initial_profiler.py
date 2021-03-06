#
# SEU detection and correction
# Copyright (C) 2015 Nano Avionics
#
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import re
import os

genDir = os.path.join('build', 'gen')
headLinkScriptTemplate = os.path.join(genDir, 'template_half_0.ld')
tailLinkScriptTemplate = os.path.join(genDir, 'template_half_1.ld')
generatedLinkerScript = os.path.join(genDir, 'secondary_seu_link.ld')
functionMap = os.path.join(genDir, 'fullMap.data')

functionEntryFormat =    '        *(.text.%s)\n'
functionPadFormat =      '        . += %s;\n'
blockFinishAndStartNew = '        . += {0};\n/****** Block {1} ******/\n'

ALIGNTO = 4
BLOCKSIZE = 13312 + 4 # bytes. Exactly equals 6656 words or 3328 dwords + CRC
PAYLOADSIZE = 13286

BLOCKTRAILER = BLOCKSIZE - PAYLOADSIZE
PROFILE1_BLOCK = 0
PROFILE2_BLOCK = 4
PROFILE3_BLOCK = 9

MIN_PROG_SIZE = PROFILE3_BLOCK * BLOCKSIZE

class FunctionData:
    def __init__(self, name, length):
        self.name = name
        self.length = length
        self.align = length % ALIGNTO
        if self.align > 0:
            self.align = ALIGNTO - self.align;

        self.bytes = length + self.align
        self.written = False

    def writeEntry(self, f):
        f.write(functionEntryFormat % self.name)
        if self.align > 0:
            f.write(functionPadFormat % self.align)
        self.written = True

fnData = {}

# Read function map file into an array
with open(functionMap, 'r') as functionFile:
    for line in functionFile:
        length, name = line.split()[0:2]
        fnData[name] = FunctionData(name, int(length))

headData = ''
with open(headLinkScriptTemplate, 'r') as headTemplateFile:
    for line in headTemplateFile:
        headData += line

tailData = ''
with open(tailLinkScriptTemplate, 'r') as tailTemplateFile:
    for line in tailTemplateFile:
        tailData += line

def getBiggestFunction(maxSize):
    values = [i for i in fnData.values() if i.bytes <= maxSize]
    if len(values) == 0:
        return None

    return max(values, key=lambda i: i.bytes).name

def writeScript():
    # Write linkerscript to file

    with open(generatedLinkerScript, 'w') as outputFile:
        for line in headData: #write first half of linker script
            outputFile.write('%s' % line)

        section1 = fnData.pop('section1_check_block')
        section1fix = fnData.pop('section1_fix_block')
        crc_calc1 = fnData.pop('crc_calc1')
        crc_check1 = fnData.pop('crc_check1')
        crc_fix1 = fnData.pop('crc_fix1')

        section2 = fnData.pop('section2_check_block')
        section2fix = fnData.pop('section2_fix_block')
        crc_calc2 = fnData.pop('crc_calc2')
        crc_check2 = fnData.pop('crc_check2')
        crc_fix2 = fnData.pop('crc_fix2')

        section3 = fnData.pop('section3_check_block')
        section3fix = fnData.pop('section3_fix_block')
        crc_calc3 = fnData.pop('crc_calc3')
        crc_check3 = fnData.pop('crc_check3')
        crc_fix3 = fnData.pop('crc_fix3')

        blknum = 0

        # block[0] contains the block count which is 4 bytes long and
        # the pointer to the start of the blocks which is 4 bytes long
        bytesAvailable = PAYLOADSIZE - 8

        def writeFunction(data):
            nonlocal bytesAvailable
            data.writeEntry(outputFile)
            bytesAvailable -= data.bytes

        writeFunction(section1)
        writeFunction(section1fix)
        writeFunction(crc_calc1)
        writeFunction(crc_check1)
        writeFunction(crc_fix1)

        while len(fnData) > 0:
            name = getBiggestFunction(bytesAvailable)
            if name:
                writeFunction(fnData.pop(name))
                continue

            # could not find a function that will fit
            if bytesAvailable == PAYLOADSIZE: # remaining functions are bigger than PAYLOADSIZE
                for elem in fnData.values():
                    print('FUNCTION TOO BIG: {length} {bytes} {name}\n' .format(**elem))
                raise Exception

            blknum += 1
            blktrailer = bytesAvailable + BLOCKTRAILER
            outputFile.write(blockFinishAndStartNew.format(blktrailer, blknum))
            bytesAvailable = PAYLOADSIZE

            if blknum == PROFILE2_BLOCK:
                writeFunction(section2)
                writeFunction(section2fix)
                writeFunction(crc_calc2)
                writeFunction(crc_check2)
                writeFunction(crc_fix2)
            elif blknum == PROFILE3_BLOCK:
                writeFunction(section3)
                writeFunction(section3fix)
                writeFunction(crc_calc3)
                writeFunction(crc_check3)
                writeFunction(crc_fix3)

        if not section2.written:
            blknum += 1
            blktrailer = bytesAvailable + BLOCKTRAILER
            outputFile.write(blockFinishAndStartNew.format(blktrailer, blknum))
            bytesAvailable = PAYLOADSIZE
            while blknum != PROFILE2_BLOCK:
                blknum += 1
                blktrailer = bytesAvailable + BLOCKTRAILER
                outputFile.write(blockFinishAndStartNew.format(blktrailer, blknum))
                bytesAvailable = PAYLOADSIZE
            writeFunction(section2)
            writeFunction(section2fix)
            writeFunction(crc_calc2)
            writeFunction(crc_check2)
            writeFunction(crc_fix2)

        if not section3.written:
            blknum += 1
            blktrailer = bytesAvailable + BLOCKTRAILER
            outputFile.write(blockFinishAndStartNew.format(blktrailer, blknum))
            bytesAvailable = PAYLOADSIZE
            while blknum != PROFILE3_BLOCK:
                blknum += 1
                blktrailer = bytesAvailable + BLOCKTRAILER
                outputFile.write(blockFinishAndStartNew.format(blktrailer, blknum))
                bytesAvailable = PAYLOADSIZE
            writeFunction(section3)
            writeFunction(section3fix)
            writeFunction(crc_calc3)
            writeFunction(crc_check3)
            writeFunction(crc_fix3)

        for line in tailData: # write second half of linkerscript
            outputFile.write('%s' % line)

    print('successfully generated linker script')

writeScript()
