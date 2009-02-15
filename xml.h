#ifndef _XML_H_
#define _XML_H_

char *					/* O - Buffer */
get_value(mxml_node_t *node,		/* I - Node to get */
	  void        *buffer,		/* I - Buffer */
	  int         buflen);		/* I - Size of buffer */

#endif //_XML_H_
