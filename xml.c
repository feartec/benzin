#include <string.h>
#include <mxml.h>

#include "types.h"
#include "xml.h"

char *					/* O - Buffer */
get_value(mxml_node_t *node,		/* I - Node to get */
         void        *buffer,		/* I - Buffer */
	 int         buflen)		/* I - Size of buffer */
{
	char		*ptr,			/* Pointer into buffer */
	*end;			/* End of buffer */
	int		len;			/* Length of node */
	mxml_node_t	*current;		/* Current node */
	
	
	ptr = (char*)buffer;
	end = (char*)buffer + buflen - 1;
	char tempbuf[256];
	current = node->child;
	for (current = node->child; current && ptr < end; current = current->next)
	{
		if (current->type == MXML_TEXT)
		{
			if (current->value.text.whitespace)
				*ptr++ = ' ';
			
			len = (int)strlen(current->value.text.string);
			if (len > (int)(end - ptr))
				len = (int)(end - ptr);
			
			memcpy(ptr, current->value.text.string, len);
			ptr += len;
		}
		else if (current->type == MXML_OPAQUE)
		{
			len = (int)strlen(current->value.opaque);
			if (len > (int)(end - ptr))
				len = (int)(end - ptr);
			
			memcpy(ptr, current->value.opaque, len);
			ptr += len;
		}
		else if (current->type == MXML_INTEGER)
		{
			sprintf(tempbuf, "%f", current->value.integer);
			len = (int)strlen(tempbuf);
			if (len > (int)(end - ptr))
				len = (int)(end - ptr);
			
			memcpy(ptr, tempbuf, len);
			ptr += len;
		}
		else if (current->type == MXML_REAL)
		{
			sprintf(tempbuf, "%f", current->value.real);
			len = (int)strlen(tempbuf);
			if (len > (int)(end - ptr))
				len = (int)(end - ptr);
			
			memcpy(ptr, tempbuf, len);
			ptr += len;
		}
	}
	*ptr = 0;
	return buffer;
}
