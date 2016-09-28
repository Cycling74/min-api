# Special Methods

## Special methods called by Max at Runtime:

* `number` : respond to both **float** and **int** messages
* `anything` : the Max catch-all method for messages of any name not otherwise defined
* `dblclick` : called when a user double-clicks on your object
* `dspsetup` : called when the Max audio signal graph is being compiled (i.e. when a user turns on the DSP for a patcher)
* `notify` : receive notifications from Max or other objects (e.g. from a `buffer~` object to which you hold a reference)


## Special methods called when constructing or wrapping your Min class:

* `setup` : a special method called when your instance is being instantiated
* `maxclass_setup` : a special method called when your class has been registered with Max
* `jitclass_setup` : for jitter classes (e.g. matrix_operators) a special method that is called when the jitter class has been registered


