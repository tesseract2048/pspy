/* Copyright (c) 2011-2013 Adam Jakubek, Rafa艂 Ga艂czy艅ski
 * Released under the MIT license (see attached LICENSE file).
 */

#include <Python.h>
#include <structmember.h>
#include <stdio.h>
#include "peer_storage.h"

typedef struct {
    PyObject_HEAD
    struct t_peer_storage * ps;
} PeerStorageObject;

static void pspy_dealloc(PeerStorageObject* self)
{
    ps_destroy(self->ps);
    PyObject_Del((PyObject*)self);
}


static PyObject* pspy_new(PyTypeObject* type,
                                PyObject* args,
                                PyObject* kwds)
{
    PeerStorageObject* self;
    self = (PeerStorageObject*)type->tp_alloc(type, 0);
    return (PyObject*)self;
}

static int pspy_init(PeerStorageObject* self,
                           PyObject* args,
                           PyObject* kwds)
{
    PyObject* obj_capacity = NULL;
    PyObject* obj_info_size = NULL;
    PyObject* obj_expire = NULL;
    uint32_t capacity, info_size, expire;
    if (!PyArg_UnpackTuple(args, "__init__", 1, 3, &obj_capacity, &obj_info_size, &obj_expire))
        return -1;
    if (obj_capacity == NULL) {
        PyErr_SetString((PyObject *)self, "capacity must be specified");
        PyErr_Print();
        return -1;
    }
    if (obj_info_size == NULL) {
        PyErr_SetString((PyObject *)self, "info_size must be specified");
        PyErr_Print();
        return -1;
    }
    if (obj_expire == NULL) {
        PyErr_SetString((PyObject *)self, "expire must be specified");
        PyErr_Print();
        return -1;
    }
    capacity = PyLong_AsLong(obj_capacity);
    info_size = PyLong_AsLong(obj_info_size);
    expire = PyLong_AsLong(obj_expire);
    self->ps = ps_create(capacity, info_size, expire);
    return 0;
}

static PyObject* pspy_repr(PeerStorageObject* self)
{
    PyObject * str = (PyObject *)PyString_FromString("<PeerStorage>");
    if (str == NULL)
        goto str_alloc_error;
    return str;
str_alloc_error:
    Py_XDECREF(str);
    PyErr_SetString(PyExc_RuntimeError, "Failed to create string");
    return NULL;
}

static PyObject* pspy_str(PeerStorageObject* self)
{
    PyObject * str = (PyObject *)PyString_FromString("<PeerStorage>");
    if (str == NULL)
        goto str_alloc_error;
    return str;
str_alloc_error:
    Py_XDECREF(str);
    PyErr_SetString(PyExc_RuntimeError, "Failed to create string");
    return NULL;
}

static PyObject* pspy_put(PeerStorageObject* self, PyObject* args)
{
    uint8_t * infohash;
    uint8_t * peerid;
    PyObject* obj_infohash = NULL;
    PyObject* obj_peerid = NULL;
    if (!PyArg_UnpackTuple(args, "put", 1, 2, &obj_infohash, &obj_peerid))
        return NULL;
    if (obj_infohash == NULL)
        return 0;
    if (obj_peerid == NULL)
        return 0;
    infohash = (uint8_t *)PyString_AsString(obj_infohash);
    peerid = (uint8_t *)PyString_AsString(obj_peerid);
    return PyLong_FromLong(ps_put(self->ps, infohash, peerid));
}

static PyObject* pspy_size(PeerStorageObject* self, PyObject* args)
{
    return PyLong_FromLong(ps_size(self->ps));
}

static void pspy_consumer(const uint8_t infohash[20], const uint32_t slot, const void * token) {
    PyObject * dict = (PyObject *)token;
    PyObject * key = PyString_FromStringAndSize((const char*)infohash, 20);
    PyObject * value = PyDict_GetItem(dict, key);
    if (value != NULL) {
        value = PyLong_FromLong(PyLong_AsLong(value) + 1);
    } else {
        value = PyLong_FromLong(1);
    }
    PyDict_SetItem(dict, key, value);
}

static PyObject* pspy_collect(PeerStorageObject* self, PyObject* arg)
{
    PyObject * dict = PyDict_New();
    ps_collect(self->ps, pspy_consumer, dict);
    return dict;
}

static PyMethodDef pspy_methods[] =
{
    { NULL }    /* sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

static PyMethodDef PeerStorageMethods[] =
{
    { "put", (PyCFunction)pspy_put, METH_VARARGS, "put" },
    { "size", (PyCFunction)pspy_size, METH_NOARGS, "size" },
    { "collect", (PyCFunction)pspy_collect, METH_NOARGS, "collect" },
    { NULL },   /* sentinel */
};

static PyTypeObject PeerStorageType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "pspy.ps",             /* tp_name */
    sizeof(PeerStorageObject),  /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)pspy_dealloc,   /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    (reprfunc)pspy_repr,        /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    (reprfunc)pspy_str,         /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Peer storage",             /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PeerStorageMethods,         /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)pspy_init,        /* tp_init */
    0,                          /* tp_alloc */
    pspy_new,                   /* tp_new */
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef pspy_moduledef = {
    PyModuleDef_HEAD_INIT,
    "pspy",                               /* m_name */
    "High performance DHT peer storage",/* m_doc */
    -1,                                 /* m_size */
    pspy_methods,                         /* m_methods */
    NULL,                               /* m_reload */
    NULL,                               /* m_traverse */
    NULL,                               /* m_clear */
    NULL,                               /* m_free */
};

PyMODINIT_FUNC
PyInit_pspy(void)
{
    PyObject* m;
    PyType_Ready(&PeerStorageType);
    m = PyModule_Create(&pspy_moduledef);
    Py_INCREF(&PeerStorageType);
    PyModule_AddObject(module, "pspy", (PyObject*)&PeerStorageType);
    return m;
}

#else

PyMODINIT_FUNC
initpspy(void)
{
    PyObject* m;

    PyType_Ready(&PeerStorageType);
    m = Py_InitModule3("pspy", pspy_methods,
                       "High performance DHT peer storage");

    Py_INCREF(&PeerStorageType);
    PyModule_AddObject(m, "pspy", (PyObject*)&PeerStorageType);
}

#endif /* PY_MAJOR_VERSION >= 3 */
