#include <QString>
#include <QtTest>

#include "../src/filter.h"

const QString testName = "fooBar";
const QString testComments = ".*";
const QStringList testIncludes = { "*.foo", "*.bar" };
const QStringList testExcludes = { "*.~foo" };
const QString testFileName = "sample.js";

class FilterTests : public QObject
{
	Q_OBJECT

	public:
		FilterTests()
		{ }

	private slots:
		void initTestCase();
		void cleanupTestCase();
		void loadTest();
		void nameTest();
		void commentsTest();
		void includesTest();
		void excludesTest();
};

void FilterTests::initTestCase()
{
	QJsonDocument doc;

	QJsonObject rootObj;
	rootObj.insert("name", testName);
	rootObj.insert("comments", testComments);
	rootObj.insert("includes", QJsonValue::fromVariant(QVariant::fromValue(testIncludes)));
	rootObj.insert("excludes", QJsonValue::fromVariant(QVariant::fromValue(testExcludes)));
	doc.setObject(rootObj);

	QFile file(testFileName);
	if(file.open(QFile::ReadWrite | QIODevice::Truncate)) {
		file.write(doc.toJson());
		file.close();
	}
}

void FilterTests::cleanupTestCase()
{
	QFile::remove(testFileName);
}

void FilterTests::loadTest()
{
	Filter filter(testFileName);
	QCOMPARE(filter.name(), testName);
	QCOMPARE(filter.comments().pattern(), testComments);
	QCOMPARE(filter.includes(), testIncludes);
	QCOMPARE(filter.excludes(), testExcludes);
}

void FilterTests::nameTest()
{
	const QString name = "barBaz";

	Filter filter(testFileName);
	filter.setName(name);

	QCOMPARE(filter.name(), name);
}

void FilterTests::commentsTest()
{
	const QString comments = "^foo$";

	Filter filter(testFileName);
	filter.setComments(QRegularExpression(comments));

	QCOMPARE(filter.comments().pattern(), comments);
}

void FilterTests::includesTest()
{
	const QStringList includes = { "*.baz" };

	Filter filter(testFileName);
	filter.setIncludes(includes);

	QCOMPARE(filter.includes(), includes);
}

void FilterTests::excludesTest()
{
	const QStringList excludes = { "*.bar" };

	Filter filter(testFileName);
	filter.setExcludes(excludes);

	QCOMPARE(filter.excludes(), excludes);
}

QTEST_APPLESS_MAIN(FilterTests)

#include "filtertests.moc"
