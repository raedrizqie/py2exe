#include "Python.h"
#include <windows.h>

static char module_doc[] =
"Importer which can load extension modules from memory";

#include "MemoryModule.h"

static PyObject *
import_module(PyObject *self, PyObject *args)
{
	char *data;
	int size;
	char *initfuncname;
	HMEMORYMODULE hmem;
	FARPROC do_init;

	if (!PyArg_ParseTuple(args, "s#s:import_module", &data, &size, &initfuncname))
		return NULL;
	hmem = MemoryLoadLibrary(data);
	if (!hmem) {
		PyErr_SetString(PyExc_ImportError, "MemoryLoadLibrary failed");
		return NULL;
	}
	do_init = MemoryGetProcAddress(hmem, initfuncname);
	if (!do_init) {
		MemoryFreeLibrary(hmem);
		PyErr_Format(PyExc_ImportError,
				"Could not find function %s", initfuncname);
	}
	do_init();
	/* Retrieve from sys.modules */
	return PyImport_ImportModule(initfuncname + 4);
}

static PyObject *
get_verbose_flag(PyObject *self, PyObject *args)
{
	return PyInt_FromLong(Py_VerboseFlag);
}

static PyMethodDef methods[] = {
	{ "import_module", import_module, METH_VARARGS,
	  "import_module(code, initfunc) -> module" },
	{ "get_verbose_flag", get_verbose_flag, METH_NOARGS,
	  "Return the Py_Verbose flag" },
	{ NULL, NULL },		/* Sentinel */
};

DL_EXPORT(void)
init_memimporter(void)
{
	Py_InitModule3("_memimporter", methods, module_doc);
}