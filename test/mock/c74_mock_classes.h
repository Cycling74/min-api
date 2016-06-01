//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
namespace max {


/**	A mocked messlist implementation.	
	Unlike the real Max messlist this uses an STL hashtab container to manage all of memory and accessors.
 */
typedef std::unordered_map<std::string, method>	t_mock_messlist;



/**	Initializes a class by informing Max of its name, instance creation and free functions, size and argument types.
	This mocks the behavior of Max's real class_addmethod().

	@ingroup classes
	@param 	name	The class's name, as a C-string
	@param 	mnew	The instance creation function
	@param 	mfree	The instance free function
	@param 	size	The size of the object's data structure in bytes.
	@param 	mmenu	Obsolete - pass NULL.
	@param 	type	A NULL-terminated list of types for the constructor.
	@return 		This function returns the class pointer for the new object class.

	@remark			The internal messlist implementation for the mock t_class is slightly different than that of the real Max.
					Generally this should not trip anyone up, but in the specific case where the classname is expect to be in the 
					messlist at index -1 there will be problems as this messlist is a hashtab and not an array.
					The code for this function and object_classname() should make be self-evident for how to accomplish this using the mock t_class.
 */
t_class* class_new(const char* name, const method mnew, const method mfree, long size, const method mmenu, short type, ...)
//inline t_class *class_new(C74_CONST char *name, C74_CONST method mnew, C74_CONST method mfree, long size, C74_CONST method mmenu, short type, ...)
{
	t_class			*c = new t_class;
	t_mock_messlist	*mock_messlist = new t_mock_messlist;
	
	c->c_sym = gensym(name);
	c->c_freefun = mfree;
	c->c_size = size;
	
	(*mock_messlist)["###CLASS###"] = (method)c;
	(*mock_messlist)["classname"] = (method)object_classname;
	c->c_messlist = (t_messlist*)mock_messlist;
	
	return c;
}


/**	Add a class to the class registry.
	This should mock the behavior of Max's real class_register() but currently does nothing at all!
 
	@ingroup	classes
	@param		name_space	Typically "box".
	@param		c			The class to register.
	@return					An error code.
 
	@remark		For an implementation that works with object_new() et. al. we need to actually implement something here.
				The way it works in Max itself is that we have a registry at global scope.
				Ideally we could implement this non-globally, as suggested also in the code surrounding the mock gensym() implementation.
 */
t_max_err class_register(t_symbol *name_space, t_class *c) { return MAX_ERR_NONE; }


/**	Add a method to a #t_class.
	This mocks the behavior of Max's real class_addmethod().
	
	@ingroup	classes
	@param		c		The class to which to add the message binding.
	@param		m		The method to which the message maps.
	@param		name	The message name.
	@param		...		A list of types -- NOT CURRENTLY IMPLEMENTED
	@return				A Max error code.
 
	@seealso	class_new()
 */
t_max_err class_addmethod(t_class *c, const method m, const char *name, ...)
{
	t_mock_messlist *mock_messlist = (t_mock_messlist*)c->c_messlist;
	(*mock_messlist)[name] = m;
	return 0;
}

}} // namespace c74::max

#include "c74_mock_inlets.h"
#include "c74_mock_outlets.h"

namespace c74 {
namespace max {


/**	Create an instance of a #t_class.
	This mocks the behavior of Max's real object_alloc().
 
	@ingroup	classes
	@param	c	The class of which to create an instance.
	@return		A pointer to the instance.
 
	@remark		At the moment this implementation does not know about fancy obex stuff!
 */
void *object_alloc(t_class *c)
{
	t_object *o = (t_object*)sysmem_newptrclear(c->c_size);
	
	o->o_messlist = c->c_messlist;
	o->o_magic = OB_MAGIC;
	o->o_inlet = (t_inlet*) new object_inlets(o);
	{
		t_mock_outlets	mock_outlets;

		// outlets are accessed through a global hash rather than by this struct member
		// this is because many outlet calls do not include the t_object pointer!
		// o->o_outlet = (struct outlet*) new t_mock_outlets;
		g_object_to_outletset[o] = mock_outlets;
	}
	return o;
}


/**	Free an instance of a #t_class.
	This mocks the behavior of Max's real object_alloc().
 
	 @ingroup	classes
	 @param	x	The pointer to the object to free.
	 @return	An error code.
	 
	 @remark	At the moment, we don't know about tinyobjects, should be easy to add support for that.
 */
t_max_err object_free(void *x)
{
	if (x) {
		t_object		*o = (t_object*)x;
		t_mock_messlist *mock_messlist = (t_mock_messlist*)o->o_messlist;
		t_class			*c = (t_class *) ((*mock_messlist)["###CLASS###"]);
		
		if (c->c_freefun)
			(*c->c_freefun)(x);
		o->o_magic = -1;
		
		delete (object_inlets*)o->o_inlet;
		g_object_to_outletset.erase(o);
		
		sysmem_freeptr(x);
	}
	return MAX_ERR_NONE;
}


/**	Return the name of the class represented by a #t_object instance.
	This mocks the behavior of Max's real object_classname(). 
 */
t_symbol *object_classname(t_object *x)
{
	t_object		*o = (t_object*)x;
	t_mock_messlist *mock_messlist = (t_mock_messlist*)o->o_messlist;
	t_class			*c = (t_class *) ((*mock_messlist)["###CLASS###"]);
	
	return c->c_sym;
}



method zgetfn(t_object *op, t_symbol *msg)
{
	t_mock_messlist *messlist = (t_mock_messlist*)op->o_messlist;

	return (*messlist)[msg->s_name];
}


method object_method_direct_getmethod(t_object *x, t_symbol *sym)
{
	// TODO: This function should be an obex-enhanced version of zgetfn(), but the mock implementation is currently obex crippled.
	return zgetfn(x, sym);
}


t_object *object_method_direct_getobject(t_object *x, t_symbol *sym)
{
	// TODO: once again, the mock implementation is not currently obex-savvy
	return x;
}


}} // namespace c74::max

