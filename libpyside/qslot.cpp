/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <shiboken.h>
#include <dynamicqmetaobject.h>

#include <QString>

#define SLOT_DEC_NAME "slot"

typedef struct
{
    PyObject_HEAD
    char* slotName;
    char* args;
    char* resultType;
} SlotData;

extern "C"
{

static int slot_init(PyObject*, PyObject*, PyObject*);
static PyObject* slot_call(PyObject*, PyObject*, PyObject*);

//aux
static char* slot_get_type_name(PyObject*);


// Class Definition -----------------------------------------------
static PyTypeObject Slot_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "QtCore."SLOT_DEC_NAME,    /*tp_name*/
    sizeof(SlotData),          /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    slot_call,                /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    SLOT_DEC_NAME,             /*tp_doc */
    0,                         /*tp_traverse */
    0,                         /*tp_clear */
    0,                         /*tp_richcompare */
    0,                         /*tp_weaklistoffset */
    0,                         /*tp_iter */
    0,                         /*tp_iternext */
    0,                         /*tp_methods */
    0,                         /*tp_members */
    0,                         /*tp_getset */
    0,                         /*tp_base */
    0,                         /*tp_dict */
    0,                         /*tp_descr_get */
    0,                         /*tp_descr_set */
    0,                         /*tp_dictoffset */
    (initproc)slot_init,      /*tp_init */
    0,                         /*tp_alloc */
    PyType_GenericNew,         /*tp_new */
    0,                         /*tp_free */
    0,                         /*tp_is_gc */
    0,                         /*tp_bases */
    0,                         /*tp_mro */
    0,                         /*tp_cache */
    0,                         /*tp_subclasses */
    0,                         /*tp_weaklist */
    0,                         /*tp_del */
};

void init_slot(PyObject *module)
{
    if (PyType_Ready(&Slot_Type) < 0)
        return;

    Py_INCREF(&Slot_Type);
    PyModule_AddObject(module, SLOT_DEC_NAME, ((PyObject*)&Slot_Type));
}


} // extern "C"

char* slot_get_type_name(PyObject *type)
{
    if (PyType_Check(type)) {
        //tp_name return the full name
        Shiboken::AutoDecRef typeName(PyObject_GetAttrString(type, "__name__"));
        return strdup(PyString_AS_STRING(typeName.object()));
    } else if (PyString_Check(type)) {
        return strdup(PyString_AS_STRING(type));
    }
    return 0;
}

int slot_init(PyObject *self, PyObject *args, PyObject *kw)
{
    static PyObject *emptyTuple = 0;
    static const char *kwlist[] = {"name", "result", 0};
    char* argName = 0;
    PyObject* argResult = 0;

    if (emptyTuple == 0)
        emptyTuple = PyTuple_New(0);

    if (!PyArg_ParseTupleAndKeywords(emptyTuple, kw, "|sO:QtCore."SLOT_DEC_NAME, (char**) kwlist, &argName, &argResult))
        return 0;

    SlotData *data = reinterpret_cast<SlotData*>(self);
    for(Py_ssize_t i = 0, i_max = PyTuple_Size(args); i < i_max; i++) {
        PyObject *argType = PyTuple_GET_ITEM(args, i);
        char *typeName = slot_get_type_name(argType);
        if (typeName) {
            if (data->args) {
                data->args = reinterpret_cast<char*>(realloc(data->args, (strlen(data->args) + 1 + strlen(typeName)) * sizeof(char*)));
                data->args = strcat(data->args, ",");
                data->args = strcat(data->args, typeName);
                free(typeName);
            } else {
                data->args = typeName;
            }
        }
    }

    if (argName)
        data->slotName = strdup(argName);

    if (argResult)
        data->resultType = slot_get_type_name(argResult);
    else
        data->resultType = strdup("void");

    return 1;
}

PyObject* slot_call(PyObject* self, PyObject* args, PyObject* kw)
{
    static PyObject* pySlotName = 0;
    PyObject* callback;
    callback = PyTuple_GetItem(args, 0);
    Py_INCREF(callback);

    if (PyFunction_Check(callback)) {
        SlotData *data = reinterpret_cast<SlotData*>(self);

        if (!data->slotName) {
            PyObject *funcName = reinterpret_cast<PyFunctionObject*>(callback)->func_name;
            data->slotName = strdup(PyString_AS_STRING(funcName));
        }

        QString signature;
        signature.sprintf("%s %s(%s)", data->resultType, data->slotName, data->args);

        if (!pySlotName)
            pySlotName = PyString_FromString(PYSIDE_SLOT_LIST_ATTR);

        PyObject *pySignature = PyString_FromString(QMetaObject::normalizedSignature(signature.toAscii()));
        PyObject *signatureList = 0;
        if (PyObject_HasAttr(callback, pySlotName)) {
            signatureList = PyObject_GetAttr(callback, pySlotName);
        } else {
            signatureList = PyList_New(0);
            PyObject_SetAttr(callback, pySlotName, signatureList);
            Py_DECREF(signatureList);
        }

        PyList_Append(signatureList, pySignature);
        Py_DECREF(pySignature);

        //clear data
        free(data->slotName);
        data->slotName = 0;
        free(data->resultType);
        data->resultType = 0;
        free(data->args);
        data->args = 0;
        return callback;
    }
    return callback;
}

