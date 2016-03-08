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
BLOCKSIZE = 13312 # bytes. Exactly equals 6656 words or 3328 dwords.
PAYLOADSIZE = 13286

BLOCKTRAILER = BLOCKSIZE - PAYLOADSIZE
PROFILE0_BLOCK = 0
PROFILE1_BLOCK = 4
PROFILE2_BLOCK = 9

MIN_PROG_SIZE = PROFILE2_BLOCK * BLOCKSIZE

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

        section1 = fnData.pop('section1_profile_func_enter')
		section1fix = fnData.pop('section1_fix_block')
        section2 = fnData.pop('section2_profile_func_enter')
		section2fix = fnData.pop('section2_fix_block')
        section3 = fnData.pop('section3_profile_func_enter')
		section3fix = fnData.pop('section3_fix_block')
        blknum = 0

        section1.writeEntry(outputFile)
        section1fix.writeEntry(outputFile)

        # block[0] contains the block count which is 4 bytes long
        bytesAvailable = PAYLOADSIZE - 4 - section1.bytes - section1fix.bytes

        def writeFunction(data):
            nonlocal bytesAvailable
            data.writeEntry(outputFile)
            bytesAvailable -= data.bytes

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
            elif blknum == PROFILE3_BLOCK:
                writeFunction(section3)
                writeFunction(section3fix)

        if not section2.written:
            outputFile.write(functionPadFormat % (((PROFILE2_BLOCK - blknum) * BLOCKSIZE) - section2.bytes - section2fix.bytes))
            writeFunction(section2)
            writeFunction(section2fix)

        if not section3.written:
            outputFile.write(functionPadFormat % (((PROFILE3_BLOCK - blknum) * BLOCKSIZE) - section3.bytes - section3fix.bytes))
            writeFunction(section3)
            writeFunction(section3fix)

        for line in tailData: # write second half of linkerscript
            outputFile.write('%s' % line)

    print('successfully generated linker script')

writeScript()
