/*
 * updatethread.h
 *
 *  Created on: Oct 20, 2009
 *      Author: Sergei Stolyarov
 */

#ifndef UPDATETHREAD_H_
#define UPDATETHREAD_H_

#include <QThread>
#include <QString>

struct QStringList;

class UpdateThread : public QThread
{
	Q_OBJECT
signals:
	void progressPercentChanged(int percent);
	void progressMessageChanged(const QString & message);

public:
	enum UpdateThreadError
	{
		NoError,
		GetCollectionsListError,        // unable to get collections list
		CannotCreateTemporaryDirsTable, //

	};

	enum ReturnAction
	{
		Continue,  // continue operations
		Terminate, // terminate with rollback
		Break      // break with commit
	};

	UpdateThread(QObject *parent = 0);
	~UpdateThread();

	/**
	 * Post STOP message to the thread, thread should correctly terminate without
	 * damaging the database.
	 */
	void softStop();
	QString errorMessage();
	int errorCode();

protected:
	void run();
	ReturnAction processCollection(const QStringList &);

private:
	struct Private;
	Private * p;
};

#endif /* UPDATETHREAD_H_ */
