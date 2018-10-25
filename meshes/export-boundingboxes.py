#!/usr/bin/env python

#based on 'export-sprites.py' and 'glsprite.py' from TCHOW Rainbow; code used is released into the public domain.

#Note: Script meant to be executed from within blender, as per:
#blender --background --python export-boundingboxes.py -- <infile.blend>[:layer] <outfile.bbx>

import sys, re

args = []
for i in range(0,len(sys.argv)):
	if sys.argv[i] == '--':
		args = sys.argv[i+1:]

if len(args) != 2:
	print("\n\nUsage:\nblender --background --python export-boundingboxes.py -- <infile.blend>[:layer] <outfile.bbx>\nExports the bounding boxes of meshes referenced by all objects in layer (default 1) to a binary blob, indexed by the names of the objects that reference them.\n")
	exit(1)

infile = args[0]
layer = 1
m = re.match(r'^(.*):(\d+)$', infile)
if m:
	infile = m.group(1)
	layer = int(m.group(2))
outfile = args[1]

assert layer >= 1 and layer <= 20

print("Will export bounding boxes referenced from layer " + str(layer) + " of '" + infile + "' to '" + outfile + "'.")

filetype = ".bbx"

import bpy
import struct

import argparse


bpy.ops.wm.open_mainfile(filepath=infile)

#meshes to write:
to_write = set()
for obj in bpy.data.objects:
	if obj.layers[layer-1] and obj.type == 'MESH':
		to_write.add(obj.data)

#strings contains the mesh names:
strings = b''

#index gives offsets into the data (and names) for each mesh:
index = b''

#bounding boxes
bbx = b''

vertex_count = 0
for obj in bpy.data.objects:
	if obj.data in to_write:
		to_write.remove(obj.data)
	else:
		continue

	mesh = obj.data
	name = mesh.name

	print("Writing '" + name + "'...")
	if bpy.context.mode == 'EDIT':
		bpy.ops.object.mode_set(mode='OBJECT') #get out of edit mode (just in case)

	#make sure object is on a visible layer:
	bpy.context.scene.layers = obj.layers
	#select the object and make it the active object:
	bpy.ops.object.select_all(action='DESELECT')
	obj.select = True
	bpy.context.scene.objects.active = obj

	#apply all modifiers (?):
	bpy.ops.object.convert(target='MESH')

	#record mesh name, start position and vertex count in the index:
	name_begin = len(strings)
	strings += bytes(name, "utf8")
	name_end = len(strings)
	index += struct.pack('I', name_begin)
	index += struct.pack('I', name_end)

	index += struct.pack('I', vertex_count) #vertex_begin
	#...count will be written below

	for i in [0, 4, 5, 1]:  # corresponds to p0, p1, p2, p3 of BoundingBox
		for j in [0, 2]:    # export only x, z
			bbx += struct.pack('f', obj.bound_box[i][j])
	vertex_count += 4  # 4 vertices for one bbx

	index += struct.pack('I', vertex_count) #vertex_end

#check that we wrote as much data as anticipated:
assert(vertex_count * 2 * 4 == len(bbx))

#write the data chunk and index chunk to an output blob:
blob = open(outfile, 'wb')
#first chunk: the bbx
blob.write(struct.pack('4s', b'bbx0')) #type
blob.write(struct.pack('I', len(bbx))) #length
blob.write(bbx)
#second chunk: the strings
blob.write(struct.pack('4s',b'str0')) #type
blob.write(struct.pack('I', len(strings))) #length
blob.write(strings)
#third chunk: the index
blob.write(struct.pack('4s',b'idx0')) #type
blob.write(struct.pack('I', len(index))) #length
blob.write(index)
wrote = blob.tell()
blob.close()

print("Wrote " + str(wrote) + " bytes [== " + str(len(bbx)+8) + " bytes of data + " + str(len(strings)+8) + " bytes of strings + " + str(len(index)+8) + " bytes of index] to '" + outfile + "'")