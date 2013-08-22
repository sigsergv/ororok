/*
 * application.h
 *
 *  Created on: Nov 12, 2009
 *      Author: Sergey Stolyarov
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

struct QString;

namespace Ororok
{

void warning(const QString & title, const QString & message);
void critical(const QString & title, const QString & message);

}

#endif /* APPLICATION_H_ */
