#ifndef SMARTPTR_H
#define SMARTPTR_H

class ReferencedObj{
	public:
		ReferencedObj(void *object):m_refcount(1),m_object(object){}
		void inc(){m_refcount++;}
		void dec(){m_refcount--;}
		int count(){return m_refcount;}
		void *object(){return m_object;}
	private:
		int m_refcount;
		void *m_object;
};

template<class T> class SmartPtr{
	template<typename> friend class SmartPtr;
	public:
		SmartPtr(T *object=NULL){
			m_refobj=new ReferencedObj(object);
		}
		SmartPtr(const SmartPtr<T> &r){
			m_refobj=r.m_refobj;
			if (m_refobj)
				m_refobj->inc();
		}
		template<class U> SmartPtr(const SmartPtr<U> &r){
			m_refobj=r.m_refobj;
			if (m_refobj)
				m_refobj->inc();
		}
		~SmartPtr<T>(){
			if (m_refobj){
				m_refobj->dec();
				if (m_refobj->count()==0){
					if (m_refobj->object())
						delete static_cast<T*>(m_refobj->object());
					delete m_refobj;
					m_refobj=NULL;
				}
			}
		}
		T &operator*(){
			return *(static_cast<T*>(m_refobj->object()));
		}
		T *operator->(){
			return static_cast<T*>(m_refobj->object());
		}
		operator bool(){
			return m_refobj->object()!=NULL;
		}
		SmartPtr<T> &operator=(const SmartPtr<T> &r){
			if (this==&r)
				return *this;
			if (r.m_refobj->object())
				r.m_refobj->inc();
			if (m_refobj){
				m_refobj->dec();
				if (m_refobj->count()==0){
					if (m_refobj->object())
						delete static_cast<T*>(m_refobj->object());
					delete m_refobj;
					m_refobj=NULL;
				}
			}
			m_refobj=r.m_refobj;
			return *this;
		}
		bool operator==(const SmartPtr<T> &r) const{
			return m_refobj==r.m_refobj;
		}
	private:
        ReferencedObj *m_refobj;
};

#endif //SMARTPTR_H
